/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class Box_Base
{
	override void EEInit()
	{
		super.EEInit();

		if (GetType().IndexOf("AmmoBox") != -1)
			MetricZ_Storage.s_AmmoBoxes.Inc();
		else
			MetricZ_Storage.s_Boxes.Inc();
	}

	override void EEDelete(EntityAI parent)
	{
		if (GetType().IndexOf("AmmoBox") != -1)
			MetricZ_Storage.s_AmmoBoxes.Dec();
		else
			MetricZ_Storage.s_Boxes.Dec();

		super.EEDelete(parent);
	}
}
#endif
