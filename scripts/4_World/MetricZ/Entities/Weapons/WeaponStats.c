/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Weapon shot and live-count aggregator.
    \details
      - counts total shots and per-weapon shots
      - tracks live weapons count per canonical type
      Weapon key is canonicalized and lowercased via Weapon_Base::MetricZ_GetLabelTypeName().
*/
class MetricZ_WeaponStats
{
	// weapon -> total shots
	protected static ref map<string, int> s_ShotsByWeapon = new map<string, int>();
	// weapon -> live count in world
	protected static ref map<string, int> s_CountByType = new map<string, int>();
	// weapon -> cached labels "{world=...,host=...,instance_id=...,weapon=...}"
	protected static ref map<string, string> s_LabelsByWeapon = new map<string, string>();

	// metrics
	protected static ref MetricZ_MetricInt s_MetricShotsByType = new MetricZ_MetricInt(
	    "weapon_shots",
	    "Shots by weapon base type",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricShootsTotal = new MetricZ_MetricInt(
	    "weapon_shots_all",
	    "Total shots on server",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricCountByType = new MetricZ_MetricInt(
	    "weapons_by_type",
	    "Weapons in world grouped by canonical type",
	    MetricZ_MetricType.GAUGE);

	/**
	    \brief Increment counters for a fired weapon.
	*/
	static void OnFire(Weapon_Base wpn)
	{
		if (!wpn)
			return;

		string type = wpn.MetricZ_GetLabelTypeName();

		int v;
		if (s_ShotsByWeapon.Find(type, v))
			s_ShotsByWeapon.Set(type, v + 1);
		else {
			s_ShotsByWeapon.Insert(type, 1);
			// build labels on first sight of this type
			LabelsFor(type);
		}

		s_MetricShootsTotal.Inc();
	}

	/**
	    \brief Increment per-type live count for spawned weapon.
	*/
	static void OnSpawn(Weapon_Base weapon)
	{
		if (!weapon)
			return;

		string type = weapon.MetricZ_GetLabelTypeName();

		int v;
		if (s_CountByType.Find(type, v))
			s_CountByType.Set(type, v + 1);
		else {
			s_CountByType.Insert(type, 1);
			// build labels on first sight of this type
			LabelsFor(type);
		}
	}

	/**
	    \brief Decrement per-type live count for deleted weapon.
	*/
	static void OnDelete(Weapon_Base weapon)
	{
		if (!weapon)
			return;

		string type = weapon.MetricZ_GetLabelTypeName();

		int v;
		if (!s_CountByType.Find(type, v))
			return;

		v = v - 1;
		if (v <= 0)
			s_CountByType.Remove(type);
		else
			s_CountByType.Set(type, v);
	}

	/**
	    \brief Get or build cached labels for a weapon type.
	    \details Always returns non-empty label block with base labels and "weapon".
	*/
	protected static string LabelsFor(string type)
	{
		string labels;
		if (s_LabelsByWeapon && s_LabelsByWeapon.Find(type, labels) && labels != string.Empty)
			return labels;

		map<string, string> m = new map<string, string>();
		m.Insert("weapon", type);
		labels = MetricZ_LabelUtils.MakeLabels(m);
		if (!s_LabelsByWeapon)
			s_LabelsByWeapon = new map<string, string>();
		s_LabelsByWeapon.Set(type, labels);
		return labels;
	}

	/**
	    \brief Flush all weapon metrics (shots + live counts).
	    \param fh Open file handle
	*/
	static void Flush(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif
		if (!fh)
			return;

		bool hasShots = (s_ShotsByWeapon.Count() > 0);
		bool hasCounts = (s_CountByType.Count() > 0);

		if (!hasShots && !hasCounts)
			return;

		// total shots + per-weapon shots
		if (hasShots) {
			// total
			s_MetricShootsTotal.FlushWithHead(fh, MetricZ_Storage.GetLabels());

			// per-weapon series
			s_MetricShotsByType.WriteHeaders(fh);
			foreach (string key, int val : s_ShotsByWeapon) {
				s_MetricShotsByType.Set(val);
				s_MetricShotsByType.Flush(fh, LabelsFor(key));
			}
		}

		// live weapons per type
		if (hasCounts) {
			s_MetricCountByType.WriteHeaders(fh);

			foreach (string key2, int cnt : s_CountByType) {
				s_MetricCountByType.Set(cnt);
				s_MetricCountByType.Flush(fh, LabelsFor(key2));
			}
		}

#ifdef DIAG
		ErrorEx(
		    "MetricZ weapon_shots / weapons_by_type scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s",
		    ErrorExSeverity.INFO);
#endif
	}
}
#endif
