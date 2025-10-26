/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup and kills.
*/
modded class AnimalBase
{
	/**
	    \brief Increment animal gauge on entity init.
	*/
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Animals.Inc();
	}

	/**
	    \brief Decrement animal gauge on entity delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Animals.Dec();

		super.EEDelete(parent);
	}

	/**
	    \brief Increment animal deaths counter on kill.
	*/
	override void EEKilled(Object killer)
	{
		MetricZ_Storage.s_AnimalsDeaths.Inc();

		super.EEKilled(killer);
	}
}
#endif
