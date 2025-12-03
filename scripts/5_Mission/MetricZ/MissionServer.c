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
	    \brief Count new player spawns.
	    \return \p PlayerBase Created player
	*/
	override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
	{
		super.OnClientNewEvent(identity, pos, ctx);

		if (m_player)
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
}
#endif
