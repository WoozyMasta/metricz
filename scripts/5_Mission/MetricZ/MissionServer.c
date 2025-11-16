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
	protected ref MetricZ_Exporter m_MetricZ;

	/**
	    \brief Initialize MetricZ and schedule first scrape.
	*/
	override void OnInit()
	{
		m_MetricZ = MetricZ_Exporter.Get();

		super.OnInit();
	}

	/**
	    \brief Called on mission finish to shutdown MetricZ.
	*/
	override void OnMissionFinish()
	{
		if (m_MetricZ)
			m_MetricZ.Shutdown();

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
}
#endif
