/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class TerritoryFlag
{
	protected ref MetricZ_TerritoryMetrics m_MetricZ;

	/**
	    \brief Track territory flag and register metrics collector.
	    \details Increments gauge, registers in registry unless disabled.
	*/
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_TerritoryFlags.Inc();

		if (MetricZ_Config.s_DisableTerritoryMetrics)
			return;

		MetricZ_TerritoryRegistry.Register(this);
		if (!m_MetricZ) {
			m_MetricZ = new MetricZ_TerritoryMetrics();
			m_MetricZ.Init(this);
		}
	}

	/**
	    \brief Unregister metrics and decrement gauge on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (!MetricZ_Config.s_DisableTerritoryMetrics) {
			m_MetricZ = null;
			MetricZ_TerritoryRegistry.Unregister(this);
		}

		MetricZ_Storage.s_TerritoryFlags.Dec();

		super.EEDelete(parent);
	}

	/**
	    \brief Accessor for per-territory metrics.
	    \return \p MetricZ_TerritoryMetrics or null.
	*/
	MetricZ_TerritoryMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}
}
#endif
