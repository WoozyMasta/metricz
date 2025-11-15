/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup and kills.
*/
modded class HelicopterScript
{
	protected ref MetricZ_TransportMetrics m_MetricZ;

	/**
	    \brief Initialize transport metrics for helicopter loaded from persistence.
	*/
	override void EEOnAfterLoad()
	{
		super.EEOnAfterLoad();

		if (MetricZ_Config.s_DisableTransportMetrics)
			return;

		if (!m_MetricZ)
			m_MetricZ = new MetricZ_TransportMetrics();

		// Init metrics for a persistent transport loaded from save with the actual persistence hash.
		m_MetricZ.Init(this);
	}

	/**
	    \brief Register helicopter in transport registry and create metrics.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (MetricZ_Config.s_DisableTransportMetrics)
			return;

		MetricZ_Storage.s_Helicopters.Inc();
		MetricZ_TransportRegistry.Register(this);

		if (!m_MetricZ)
			m_MetricZ = new MetricZ_TransportMetrics();

		// Scheduled init of metrics for the created transport.
		// In this state, the persistence hash is not guaranteed and must be loaded later.
		// However, if the transport was not loaded but created via debug, this is the only reliable place for integration.
		m_MetricZ.InitLater(this);
	}

	/**
	    \brief Unregister helicopter and decrement gauges on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics) {
			m_MetricZ = null;
			MetricZ_Storage.s_Helicopters.Dec();
			MetricZ_TransportRegistry.Unregister(this);
		}

		super.EEDelete(parent);
	}

	/**
	    \brief Increment helicopters destroyed counter on kill if enabled.
	*/
	override void EEKilled(Object killer)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics)
			MetricZ_Storage.s_HelicoptersDestroys.Inc();

		super.EEKilled(killer);
	}

	/**
	    \brief Accessor for per-helicopter metrics.
	    \return \p MetricZ_TransportMetrics or null.
	*/
	MetricZ_TransportMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}
}
#endif
