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
	/**
	    \brief Increment transport based counters.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (MetricZ_Config.s_DisableTransportMetrics)
			return;

		if (Expansion_IsBoat())
			MetricZ_Storage.s_Boats.Inc();
		else if (Expansion_IsHelicopter() || Expansion_IsPlane())
			MetricZ_Storage.s_Helicopters.Inc();
		else
			MetricZ_Storage.s_Cars.Inc();
	}

	/**
	    \brief Decrement transport based counters.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics) {
			if (Expansion_IsBoat())
				MetricZ_Storage.s_Boats.Dec();
			else if (Expansion_IsHelicopter() || Expansion_IsPlane())
				MetricZ_Storage.s_Helicopters.Dec();
			else
				MetricZ_Storage.s_Cars.Dec();
		}

		super.EEDelete(parent);
	}

	/**
	    \brief Increment transport based destroyed counter on kill if enabled.
	*/
	override void EEKilled(Object killer)
	{
		if (!MetricZ_Config.s_DisableTransportMetrics) {
			if (Expansion_IsBoat())
				MetricZ_Storage.s_BoatsDestroys.Inc();
			else if (Expansion_IsHelicopter() || Expansion_IsPlane())
				MetricZ_Storage.s_HelicoptersDestroys.Inc();
			else
				MetricZ_Storage.s_CarsDestroys.Inc();
		}

		super.EEKilled(killer);
	}
}
#endif
#endif
