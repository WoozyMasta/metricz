/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup and kills.
*/
modded class CarScript
{
	// prevent counting kills on destroyed vehicles
	protected bool m_MetricZ_IsKilled;

	protected ref MetricZ_TransportMetrics m_MetricZ;

	/**
	    \brief Initialize transport metrics for car loaded from persistence.
	*/
	override void EEOnAfterLoad()
	{
		super.EEOnAfterLoad();

		if (!MetricZ_Config.IsLoaded() || MetricZ_Config.Get().disableTransportMetrics)
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
	    \brief Register car in transport registry and create metrics.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (!MetricZ_Config.IsLoaded() || MetricZ_Config.Get().disableTransportMetrics)
			return;

#ifdef EXPANSIONMODVEHICLE
		if (IsInherited(ExpansionBoatScript))
			MetricZ_Storage.s_Boats.Inc();
		else if (IsInherited(ExpansionHelicopterScript))
			MetricZ_Storage.s_Helicopters.Inc();
		else
			MetricZ_Storage.s_Cars.Inc();
#else
		MetricZ_Storage.s_Cars.Inc();
#endif

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
	    \brief Unregister car and decrement gauges on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disableTransportMetrics) {
			m_MetricZ = null;

#ifdef EXPANSIONMODVEHICLE
			if (IsInherited(ExpansionBoatScript))
				MetricZ_Storage.s_Boats.Dec();
			else if (IsInherited(ExpansionHelicopterScript))
				MetricZ_Storage.s_Helicopters.Dec();
			else
				MetricZ_Storage.s_Cars.Dec();
#else
			MetricZ_Storage.s_Cars.Dec();
#endif

			MetricZ_TransportRegistry.Unregister(this);
		}

		super.EEDelete(parent);
	}

	/**
	    \brief Increment cars destroyed counter on kill if enabled.
	*/
	override void EEKilled(Object killer)
	{
		if (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disableTransportMetrics && !m_MetricZ_IsKilled) {
#ifdef EXPANSIONMODVEHICLE
			if (IsInherited(ExpansionBoatScript))
				MetricZ_Storage.s_BoatsDestroys.Inc();
			else if (IsInherited(ExpansionHelicopterScript))
				MetricZ_Storage.s_HelicoptersDestroys.Inc();
			else
				MetricZ_Storage.s_CarsDestroys.Inc();
#else
			MetricZ_Storage.s_CarsDestroys.Inc();
#endif
			m_MetricZ_IsKilled = true;
		}

		super.EEKilled(killer);
	}

	/**
	    \brief Accessor for per-car metrics.
	    \return \p MetricZ_TransportMetrics or null.
	*/
	MetricZ_TransportMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}
}
#endif
