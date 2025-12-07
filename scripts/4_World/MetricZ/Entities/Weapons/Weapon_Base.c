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
	// cache for weapon label names: original type -> canonical name
	protected static ref map<string, string> s_MetricZ_LabelNames = new map<string, string>();

	/**
	    \brief Increment weapons gauge on init.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (!MetricZ_Config.IsLoaded())
			return;

		if (!MetricZ_Config.Get().disableWeaponMetrics)
			MetricZ_WeaponStats.OnSpawn(this);

		MetricZ_Storage.s_Weapons.Inc();
	}

	/**
	    \brief Decrement weapons gauge on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (MetricZ_Config.IsLoaded()) {
			MetricZ_Storage.s_Weapons.Dec();

			if (!MetricZ_Config.Get().disableWeaponMetrics)
				MetricZ_WeaponStats.OnDelete(this);
		}

		super.EEDelete(parent);
	}

	/**
	    \brief Count weapon shot for stats.
	    \details Calls base then forwards to MetricZ_WeaponStats unless disabled.
	*/
	override void OnFire(int muzzle_index)
	{
		super.OnFire(muzzle_index);

		if (!MetricZ_Config.IsLoaded())
			return;

		if (!MetricZ_Config.Get().disableWeaponMetrics)
			MetricZ_WeaponStats.OnFire(this);
	}

	/**
	    \brief Public helper for MetricZ: returns cached canonical label name for this weapon.
	    \details You can override this for set some beauty label name for your weapon.
	*/
	override string MetricZ_GetLabelTypeName()
	{
		return MetricZ_GetWeaponNameByType();
	}

	/**
	    \brief Compute canonical weapon name from type and cache the result.
	    \details
	      - remove any "sawedoff" token (case-insensitive, anywhere)
	      - collapse "__" -> "_", then trim leading/trailing "_"
	      - cut suffix after last "_"
	      - lowercase; fallback to original lowercase if empty
	*/
	protected string MetricZ_GetWeaponNameByType()
	{
		string type = GetType();

		// cache hit
		string cached;
		if (s_MetricZ_LabelNames && s_MetricZ_LabelNames.Find(type, cached))
			return cached;

		string result = MetricZ_ObjectName.GetName(this, true);
		result.Replace("sawedoff", "");

		// cache store
		if (!s_MetricZ_LabelNames)
			s_MetricZ_LabelNames = new map<string, string>();
		s_MetricZ_LabelNames.Set(type, result);

		return result;
	}
}
#endif
