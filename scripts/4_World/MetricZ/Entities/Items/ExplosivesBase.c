/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class ExplosivesBase
{
	/**
	    \brief Increment explosives gauge on init.
	*/
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Explosives.Inc();
	}

	/**
	    \brief Decrement explosives gauge on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Explosives.Dec();

		super.EEDelete(parent);
	}

	/**
	    \brief Increment detonations counter on explode.
	*/
	override protected void OnExplode()
	{
		super.OnExplode();

		MetricZ_Storage.s_Exploded.Inc();
	}
}
#endif
