/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class Bottle_Base
{
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Bottles.Inc();
	}

	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Bottles.Dec();

		super.EEDelete(parent);
	}
}
#endif
