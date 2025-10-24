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

	override void EEInit()
	{
		super.EEInit();

		if (MetricZ_Config.s_DisableTransportMetrics)
			return;

		MetricZ_Storage.s_Helicopters.Inc();
		MetricZ_TransportRegistry.Register(this);

		if (!m_MetricZ) {
			m_MetricZ = new MetricZ_TransportMetrics();
			m_MetricZ.Init(this);
		}
	}

	override void EEDelete(EntityAI parent)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics) {
			m_MetricZ = null;
			MetricZ_Storage.s_Helicopters.Dec();
			MetricZ_TransportRegistry.Unregister(this);
		}

		super.EEDelete(parent);
	}

	override void EEKilled(Object killer)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics)
			MetricZ_Storage.s_HelicoptersDestroys.Inc();

		super.EEKilled(killer);
	}

	ref MetricZ_TransportMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}
}
#endif
