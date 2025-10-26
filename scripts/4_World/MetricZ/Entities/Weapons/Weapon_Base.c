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
	/**
	    \brief Increment weapons gauge on init.
	*/
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Weapons.Inc();
	}

	/**
	    \brief Decrement weapons gauge on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Weapons.Dec();

		super.EEDelete(parent);
	}

	/**
	    \brief Count weapon shot for stats.
	    \details Calls base then forwards to MetricZ_WeaponStats unless disabled.
	*/
	override void OnFire(int muzzle_index)
	{
		super.OnFire(muzzle_index);

		if (!MetricZ_Config.s_DisableWeaponMetrics)
			MetricZ_WeaponStats.OnFire(this);
	}
}
#endif
