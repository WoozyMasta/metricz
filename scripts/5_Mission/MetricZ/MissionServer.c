/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Mission hooks for MetricZ lifecycle and periodic scraping.
*/
modded class MissionServer : MissionBase
{
	private bool m_MetricZ_Busy;

	/**
	    \brief Initialize MetricZ and schedule first scrape.
	*/
	override void OnInit()
	{
		super.OnInit();

		MetricZ_Config.Load();
		MetricZ_Storage.Init();
		ErrorEx("MetricZ loaded", ErrorExSeverity.INFO);

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.MetricZ_Update, MetricZ_Config.s_InitDelayMs, false);
	}

	/**
	    \brief Called on mission finish to shutdown MetricZ.
	*/
	override void OnMissionFinish()
	{
		MetricZ_Shutdown();

		super.OnMissionFinish();
	}

	/**
	    \brief Pump frame monitor each frame.
	    \param timeslice Delta time of the last frame in seconds.
	*/
	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		MetricZ_FrameMonitor.OnUpdate(timeslice);
	}

	/**
	    \brief Count new player spawns.
	    \return \p PlayerBase Created player
	*/
	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		super.CreateCharacter(identity, pos, ctx, characterName);

		MetricZ_Storage.s_PlayersSpawns.Inc();

		return m_player;
	}

	/**
	    \brief Track number of corpses after server update.
	    \details Calls base, then mirrors current count into MetricZ_Storage.s_Corpses.
	*/
	override void UpdateCorpseStatesServer()
	{
		super.UpdateCorpseStatesServer();

		MetricZ_Storage.s_Corpses.Set(m_DeadPlayersArray.Count());
	}

	/**
	    \brief Count completed artillery barrages.
	    \details Reads timer before calling base. After base logic, if artillery is enabled
	        and a cycle completed (before > m_ArtyDelay and m_ArtyBarrageTimer < before),
	        increments MetricZ_Storage.s_Artillery.
	    \param deltaTime Frame delta in seconds.
	*/
	override void RandomArtillery(float deltaTime)
	{
		float before = m_ArtyBarrageTimer;

		super.RandomArtillery(deltaTime);

		if (m_PlayArty && before > m_ArtyDelay && m_ArtyBarrageTimer < before)
			MetricZ_Storage.s_Artillery.Inc();
	}

	/**
	    \brief Periodic scrape task. Writes temp file and atomically publishes it.
	    \details Skips if a previous scrape is still running.
	*/
	void MetricZ_Update()
	{

		// Schedule next tick for minimize drift
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.MetricZ_Update, MetricZ_Config.s_ScrapeIntervalMs, false);

		if (m_MetricZ_Busy) {
			ErrorEx("MetricZ another scrape already works. You might want to set the scrape interval to twice as much.", ErrorExSeverity.WARNING);
			MetricZ_Storage.s_ScrapeSkippedTotal.Inc();
			return;
		}

		m_MetricZ_Busy = true;
		float t0 = g_Game.GetTickTime();

#ifdef DIAG
		ErrorEx("MetricZ start update on " + t0 + "s server tick", ErrorExSeverity.INFO);
#endif

		FileHandle fh = OpenFile(MetricZ_Config.TEMP, FileMode.WRITE);
		if (!fh) {
			m_MetricZ_Busy = false;
			ErrorEx("MetricZ open file " + MetricZ_Config.TEMP + " failed");
			return;
		}

		// world metrics
		MetricZ_Storage.Update();
		MetricZ_Storage.Flush(fh);

#ifdef DIAG
		ErrorEx("MetricZ base metrics scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif

		// per-player
		if (!MetricZ_Config.s_DisablePlayerMetrics)
			MetricZ_EntitiesWriter.FlushPlayers(fh);

		// per-vehicle
		if (!MetricZ_Config.s_DisableTransportMetrics)
			MetricZ_EntitiesWriter.FlushTransport(fh);

		// weapon shots
		if (!MetricZ_Config.s_DisableWeaponMetrics)
			MetricZ_WeaponStats.Flush(fh);

		// per-territory
		if (!MetricZ_Config.s_DisableTerritoryMetrics)
			MetricZ_EntitiesWriter.FlushTerritory(fh);

		// dayz game RPC inputs
		if (!MetricZ_Config.s_DisableRPCMetrics)
			MetricZ_RpcStats.Flush(fh);

		// dayz game events
		if (!MetricZ_Config.s_DisableEventMetrics)
			MetricZ_EventStats.Flush(fh);

		CloseFile(fh);

		// publish
		DeleteFile(MetricZ_Config.FILE);
		if (!CopyFile(MetricZ_Config.TEMP, MetricZ_Config.FILE))
			ErrorEx("MetricZ publish failed " + MetricZ_Config.TEMP + " -> " + MetricZ_Config.FILE);

		DeleteFile(MetricZ_Config.TEMP);

		float t1 = g_Game.GetTickTime();
		MetricZ_Storage.s_UpdateDurationSec.Set(t1 - t0);

#ifdef DIAG
		ErrorEx("MetricZ stored in " + MetricZ_Storage.s_UpdateDurationSec.Get().ToString() + "s", ErrorExSeverity.INFO);
#endif

		m_MetricZ_Busy = false;
	}

	/**
	    \brief Graceful shutdown. Publishes status=0 and stops scheduling.
	*/
	void MetricZ_Shutdown()
	{
		ErrorEx("MetricZ scrape shutting down", ErrorExSeverity.INFO);

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(this.MetricZ_Update);

		DeleteFile(MetricZ_Config.TEMP);

		FileHandle fh = OpenFile(MetricZ_Config.FILE, FileMode.WRITE);
		if (!fh)
			return;

		MetricZ_Storage.s_Status.Set(0);
		MetricZ_Storage.s_Status.FlushWithHead(fh, MetricZ_Storage.GetExtraLabels());

		CloseFile(fh);
	}
}
#endif
