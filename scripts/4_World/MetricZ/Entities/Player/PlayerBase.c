/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Update MetricZ counters on spawn/cleanup and kills, and register a per-player metrics instance.
*/
modded class PlayerBase
{
	protected ref MetricZ_PlayerMetrics m_MetricZ;

	/**
	    \brief Register per-player metrics collector on init.
	    \details No-op if player metrics disabled.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (MetricZ_Config.s_DisablePlayerMetrics)
			return;

		if (!m_MetricZ) {
			m_MetricZ = new MetricZ_PlayerMetrics();
			m_MetricZ.Init(this);
		}
	}

	/**
	    \brief Drop metrics reference on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		m_MetricZ = null;

		super.EEDelete(parent);
	}

	/**
	    \brief Drop metrics reference on delete.
	*/
	override void EEKilled(Object killer)
	{
		if (!MetricZ_Config.s_DisablePlayerMetrics)
			MetricZ_Storage.s_PlayersDeaths.Inc();

		super.EEKilled(killer);
	}

	/**
	    \brief Accessor for per-player metrics.
	    \return \p MetricZ_PlayerMetrics or null if not initialized.
	*/
	ref MetricZ_PlayerMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}
}
#endif
