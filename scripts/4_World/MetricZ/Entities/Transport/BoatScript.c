/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup and kills.
*/
modded class BoatScript
{
	protected ref MetricZ_TransportMetrics m_MetricZ;

	/**
	    \brief Register boat in transport registry and create metrics.
	    \details No-op if transport metrics disabled.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (MetricZ_Config.s_DisableTransportMetrics)
			return;

		MetricZ_Storage.s_Boats.Inc();
		MetricZ_TransportRegistry.Register(this);

		if (!m_MetricZ) {
			m_MetricZ = new MetricZ_TransportMetrics();
			m_MetricZ.Init(this);
		}
	}

	/**
	    \brief Unregister boat and decrement gauges on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics) {
			m_MetricZ = null;
			MetricZ_Storage.s_Boats.Dec();
			MetricZ_TransportRegistry.Unregister(this);
		}

		super.EEDelete(parent);
	}

	/**
	    \brief Increment boats destroyed counter on kill if enabled.
	*/
	override void EEKilled(Object killer)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics)
			MetricZ_Storage.s_BoatsDestroys.Inc();

		super.EEKilled(killer);
	}

	/**
	    \brief Accessor for per-boat metrics.
	    \return \p MetricZ_TransportMetrics or null.
	*/
	ref MetricZ_TransportMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}
}
#endif
