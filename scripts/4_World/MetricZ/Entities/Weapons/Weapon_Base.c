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

		if (!MetricZ_Config.s_DisableWeaponMetrics)
			MetricZ_WeaponStats.OnSpawn(this);

		MetricZ_Storage.s_Weapons.Inc();
	}

	/**
	    \brief Decrement weapons gauge on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Weapons.Dec();

		if (!MetricZ_Config.s_DisableWeaponMetrics)
			MetricZ_WeaponStats.OnDelete(this);

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

	/**
	    \brief Public helper for MetricZ: returns cached canonical label name for this weapon.
	    \details You can override this for set some beauty label name for your weapon.
	*/
	string MetricZ_GetWeaponName()
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

		string result = type;

		// remove all case-insensitive "sawedoff" occurrences
		string pat = "sawedoff";
		string low = result;
		low.ToLower();
		int plen = pat.Length();
		int pos = low.IndexOf(pat);
		while (pos != -1) {
			// delete substring [pos, pos+plen]
			result = result.Substring(0, pos) + result.Substring(pos + plen, result.Length() - (pos + plen));

			// recalc lowercase view
			low = result;
			low.ToLower();
			pos = low.IndexOf(pat);
		}

		// collapse repeated underscores
		while (result.Contains("__"))
			result.Replace("__", "_");

		// trim leading underscores
		while (result.Length() > 0 && result.Get(0) == "_")
			result = result.Substring(1, result.Length() - 1);

		// trim trailing underscores
		while (result.Length() > 0 && result.Get(result.Length() - 1) == "_")
			result = result.Substring(0, result.Length() - 1);

		// cut after last underscore (drop variant)
		if (result != string.Empty) {
			int last = result.LastIndexOf("_");
			if (last != -1) {
				if (last > 0)
					result = result.Substring(0, last);
				else
					result = "";
			}
		}

		// fallback
		if (result == string.Empty)
			result = type;

		result.ToLower();

		// cache store
		if (!s_MetricZ_LabelNames)
			s_MetricZ_LabelNames = new map<string, string>();
		s_MetricZ_LabelNames.Set(type, result);

		return result;
	}
}
#endif
