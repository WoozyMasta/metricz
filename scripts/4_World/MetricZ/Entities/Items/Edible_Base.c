/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class Edible_Base
{
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Food.Inc();
	}

	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Food.Dec();

		super.EEDelete(parent);
	}
}
#endif
