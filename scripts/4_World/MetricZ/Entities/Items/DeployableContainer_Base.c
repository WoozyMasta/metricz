/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class DeployableContainer_Base
{
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Containers.Inc();
	}

	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Containers.Dec();

		super.EEDelete(parent);
	}
}
#endif
