/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
#ifdef EXPANSIONMODVEHICLE
/**
    \brief Updates MetricZ counters on spawn/cleanup and kills expansion vehicles.
*/
modded class ExpansionVehicleBase
{
	// prevent counting kills on destroyed vehicles
	protected bool m_MetricZ_IsKilled;

	protected ref MetricZ_TransportMetrics m_MetricZ;

	/**
	    \brief Initialize transport metrics for expansion vehicle loaded from persistence.
	*/
	override void EEOnAfterLoad()
	{
		super.EEOnAfterLoad();

		if (!MetricZ_Config.IsLoaded() || MetricZ_Config.Get().disabled_metrics.transports)
			return;

		if (!m_MetricZ)
			m_MetricZ = new MetricZ_TransportMetrics();

		// Init metrics for a persistent transport loaded from save with the actual persistence hash.
		m_MetricZ.Init(this);

		// prevent double counting for save/load destroyed vehicle
		if (IsDamageDestroyed())
			m_MetricZ_IsKilled = true;
	}


	/**
	    \brief Increment transport based counters.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (!MetricZ_Config.IsLoaded() || MetricZ_Config.Get().disabled_metrics.transports)
			return;

		if (Expansion_IsBoat())
			MetricZ_Storage.s_Boats.Inc();
		else if (Expansion_IsHelicopter() || Expansion_IsPlane())
			MetricZ_Storage.s_Helicopters.Inc();
		else
			MetricZ_Storage.s_Cars.Inc();

		MetricZ_TransportRegistry.Register(this);

		if (!m_MetricZ)
			m_MetricZ = new MetricZ_TransportMetrics();

		// Scheduled init of metrics for the created transport.
		// In this state, the persistence hash is not guaranteed and must be loaded later.
		// However, if the transport was not loaded but created via debug, this is the only reliable place for integration.
		m_MetricZ.InitLater(this);

		// prevent double counting for save/load destroyed vehicle
		if (IsDamageDestroyed())
			m_MetricZ_IsKilled = true;
	}

	/**
	    \brief Decrement transport based counters.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disabled_metrics.transports) {
			if (Expansion_IsBoat())
				MetricZ_Storage.s_Boats.Dec();
			else if (Expansion_IsHelicopter() || Expansion_IsPlane())
				MetricZ_Storage.s_Helicopters.Dec();
			else
				MetricZ_Storage.s_Cars.Dec();

			MetricZ_TransportRegistry.Unregister(this);
		}

		super.EEDelete(parent);
	}

	/**
	    \brief Increment transport based destroyed counter on kill if enabled.
	*/
	override void EEKilled(Object killer)
	{
		if (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disabled_metrics.transports && !m_MetricZ_IsKilled) {
			if (Expansion_IsBoat())
				MetricZ_Storage.s_BoatsDestroys.Inc();
			else if (Expansion_IsHelicopter() || Expansion_IsPlane())
				MetricZ_Storage.s_HelicoptersDestroys.Inc();
			else
				MetricZ_Storage.s_CarsDestroys.Inc();

			m_MetricZ_IsKilled = true;
		}

		super.EEKilled(killer);
	}

	/**
	    \brief Accessor for per-expansion vehicle metrics.
	    \return \p MetricZ_TransportMetrics or null.
	*/
	MetricZ_TransportMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}
}
#endif
#endif
