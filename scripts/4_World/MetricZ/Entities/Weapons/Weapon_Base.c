/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class Weapon_Base
{
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Weapons.Inc();
	}

	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Weapons.Dec();

		super.EEDelete(parent);
	}

	override void OnFire(int muzzle_index)
	{
		super.OnFire(muzzle_index);

		if (!MetricZ_Config.s_DisableWeaponMetrics)
			MetricZ_WeaponStats.OnFire(this);
	}
}
#endif
