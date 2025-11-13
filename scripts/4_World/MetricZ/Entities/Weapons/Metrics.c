/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Weapon shot counters aggregator.
    \details Counts total shots and per-weapon shots. Weapon key is canonicalized and lowercased.
*/
class MetricZ_WeaponStats
{
	// weapon -> count
	protected static ref map<string, int> s_ShotsByWeapon = new map<string, int>();

	// cache for label strings to avoid rebuilding each scrape
	protected static ref map<string, string> s_LabelsByWeapon = new map<string, string>();

	// metrics
	protected static ref MetricZ_MetricInt s_ShotsByType = new MetricZ_MetricInt(
	    "weapon_shots",
	    "Shots by weapon base type",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_ShotsAll = new MetricZ_MetricInt(
	    "weapon_shots_all",
	    "Total shots on server",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Increment counters for a fired weapon.
	*/
	static void OnFire(Weapon_Base wpn)
	{
		if (!wpn)
			return;

		string type = MetricZ_LabelUtils.WeaponLabelName(wpn.GetType());

		int v;
		if (s_ShotsByWeapon.Find(type, v))
			s_ShotsByWeapon.Set(type, v + 1);
		else {
			s_ShotsByWeapon.Insert(type, 1);

			// build and cache labels for this weapon
			map<string, string> labels = new map<string, string>();
			labels.Insert("weapon", type);
			s_LabelsByWeapon.Insert(type, MetricZ_LabelUtils.MakeLabels(labels));
		}

		s_ShotsAll.Inc();
	}

	/**
	    \brief Flush all registered metrics.
	    \param fh Open file handle
	*/
	static void Flush(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif
		if (!fh)
			return;

		if (s_ShotsByWeapon.Count() == 0)
			return;

		// total
		s_ShotsAll.FlushWithHead(fh, MetricZ_Storage.GetLabels());

		// per-weapon series
		s_ShotsByType.WriteHeaders(fh);
		foreach (string key, int val : s_ShotsByWeapon) {
			s_ShotsByType.Set(val);

			// lazy label build if not cached for any reason
			string label = s_LabelsByWeapon.Get(key);
			if (label == string.Empty) {
				map<string, string> labels = new map<string, string>();
				labels.Insert("weapon", key);
				s_LabelsByWeapon.Set(key, MetricZ_LabelUtils.MakeLabels(labels));
			}

			s_ShotsByType.Flush(fh, label);
		}

#ifdef DIAG
		ErrorEx("MetricZ weapon_shots scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}
}
#endif
