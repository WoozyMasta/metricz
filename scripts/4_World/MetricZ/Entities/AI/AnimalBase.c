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
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Animals.Inc();
	}

	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Animals.Dec();

		super.EEDelete(parent);
	}

	override void EEKilled(Object killer)
	{
		MetricZ_Storage.s_AnimalsDeaths.Inc();

		super.EEKilled(killer);
	}
}
#endif
