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
	protected static bool s_CacheLoaded;

	// weapon -> total shots
	protected static ref map<string, int> s_ShotsByWeapon = new map<string, int>();
	// weapon -> live count in world
	protected static ref map<string, int> s_CountByType = new map<string, int>();
	// players kills by source (source name -> count)
	protected static ref map<string, int> s_PlayerKills = new map<string, int>();
	// creatures kills by source (source name -> count)
	protected static ref map<string, int> s_CreatureKills = new map<string, int>();

	// weapon -> cached labels "{world=...,host=...,instance_id=...,weapon=...}"
	protected static ref map<string, string> s_LabelsByWeapon = new map<string, string>();

	// metrics
	protected static ref MetricZ_MetricInt s_MetricShotsByType = new MetricZ_MetricInt(
	    "weapon_shots",
	    "Shots by weapon base type",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricCountByType = new MetricZ_MetricInt(
	    "weapons_by_type",
	    "Weapons in world grouped by canonical type",
	    MetricZ_MetricType.GAUGE);
	protected static ref MetricZ_MetricInt s_MetricPlayerKills = new MetricZ_MetricInt(
	    "player_killed_by",
	    "Count of players killed by source",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricCreatureKills = new MetricZ_MetricInt(
	    "creature_killed_by",
	    "Count of creatures (Infected/Animals/AI) killed by source",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Load cache of weapon types and killers objects for label persistency.
	*/
	static void LoadCache()
	{
		if (s_CacheLoaded || !MetricZ_Config.IsLoaded() || MetricZ_Config.Get().disableWeaponMetrics)
			return;

		s_CacheLoaded = true;

		array<string> knownWeapons = MetricZ_PersistentCache.GetKeys(MetricZ_CacheKey.WEAPON_TYPES);
		if (knownWeapons) {
			foreach (string weapon : knownWeapons) {
				if (!s_ShotsByWeapon.Contains(weapon)) {
					s_ShotsByWeapon.Insert(weapon, 0);
					LabelsFor(weapon);
				}
			}
		}

		array<string> knownKillers = MetricZ_PersistentCache.GetKeys(MetricZ_CacheKey.KILLER_OBJECT);
		foreach (string killer : knownKillers) {
			if (!s_PlayerKills.Contains(killer)) {
				s_PlayerKills.Insert(killer, 0);
				LabelsFor(killer);
			}

			if (!s_CreatureKills.Contains(killer)) {
				s_CreatureKills.Insert(killer, 0);
				LabelsFor(killer);
			}
		}
	}

	/**
	    \brief Increment counters for a fired weapon.
	*/
	static void OnFire(Weapon_Base weapon)
	{
		if (!weapon)
			return;

		string type = weapon.MetricZ_GetLabelTypeName();
		MetricZ_PersistentCache.Register(MetricZ_CacheKey.WEAPON_TYPES, type);
		IncMap(s_ShotsByWeapon, type);
	}

	/**
	    \brief Increment per-type live count for spawned weapon.
	*/
	static void OnSpawn(Weapon_Base weapon)
	{
		if (!weapon)
			return;

		string type = weapon.MetricZ_GetLabelTypeName();
		MetricZ_PersistentCache.Register(MetricZ_CacheKey.WEAPON_TYPES, type);
		IncMap(s_CountByType, type);
	}

	/**
	    \brief Called when a Player is killed.
	*/
	static void OnPlayerKilled(Object source)
	{
		if (!source)
			return;

		string type = ResolveSourceName(source);
		MetricZ_PersistentCache.Register(MetricZ_CacheKey.KILLER_OBJECT, type);
		IncMap(s_PlayerKills, type);
	}

	/**
	    \brief Called when a Creature (Zombie/Animal/eAI) is killed.
	*/
	static void OnCreatureKilled(Object source)
	{
		if (!source)
			return;

		string type = ResolveSourceName(source);
		MetricZ_PersistentCache.Register(MetricZ_CacheKey.KILLER_OBJECT, type);
		IncMap(s_CreatureKills, type);
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
	    \brief Flush all weapon metrics (shots, live counts, kills).
	    \param MetricZ_Sink sink instance
	*/
	static void Flush(MetricZ_Sink sink)
	{
		if (!sink)
			return;

#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		// total shots + per-weapon shots
		if (s_ShotsByWeapon.Count() > 0) {
			s_MetricShotsByType.WriteHeaders(sink);

			foreach (string key, int val : s_ShotsByWeapon) {
				s_MetricShotsByType.Set(val);
				s_MetricShotsByType.Flush(sink, LabelsFor(key));
			}
		}

		// live weapons per type
		if (s_CountByType.Count() > 0) {
			s_MetricCountByType.WriteHeaders(sink);

			foreach (string key2, int val2 : s_CountByType) {
				s_MetricCountByType.Set(val2);
				s_MetricCountByType.Flush(sink, LabelsFor(key2));
			}
		}

		// player kills
		if (s_PlayerKills.Count() > 0) {
			s_MetricPlayerKills.WriteHeaders(sink);

			foreach (string key3, int val3 : s_PlayerKills) {
				s_MetricPlayerKills.Set(val3);
				s_MetricPlayerKills.Flush(sink, LabelsFor(key3));
			}
		}

		// creature kills
		if (s_CreatureKills.Count() > 0) {
			s_MetricCreatureKills.WriteHeaders(sink);

			foreach (string key4, int val4 : s_CreatureKills) {
				s_MetricCreatureKills.Set(val4);
				s_MetricCreatureKills.Flush(sink, LabelsFor(key4));
			}
		}

#ifdef DIAG
		ErrorEx(
		    "MetricZ weapon_shots / weapons_by_type scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s",
		    ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Get or build cached labels for a weapon type.
	    \param type The normalized weapon/source name.
	    \return string Formatted Prometheus labels.
	*/
	protected static string LabelsFor(string type)
	{
		string labels;
		if (s_LabelsByWeapon && s_LabelsByWeapon.Find(type, labels) && labels != string.Empty)
			return labels;

		map<string, string> labelsMap = new map<string, string>();
		labelsMap.Insert("weapon", type);
		labels = MetricZ_LabelUtils.MakeLabels(labelsMap);

		if (!s_LabelsByWeapon)
			s_LabelsByWeapon = new map<string, string>();

		s_LabelsByWeapon.Set(type, labels);

		return labels;
	}

	/**
	    \brief Determines the metric key from an Object source.
	    \param source The killer Object.
	    \return string Normalized key (e.g., "akm", "zombie", "player_melee_item_heavy").
	*/
	protected static string ResolveSourceName(Object source)
	{
		if (!source)
			return "none";

		Weapon_Base wpn;
		if (Class.CastTo(wpn, source))
			return wpn.MetricZ_GetLabelTypeName();

		if (source.IsInherited(ZombieBase))
			return "zombie";

		if (source.IsInherited(AnimalBase))
			return "animal";

		if (source.IsInherited(Transport))
			return "transport";

		if (source.IsInherited(Grenade_Base))
			return "grenade";

		if (source.IsInherited(TrapBase))
			return "trap";

		if (source.IsInherited(ToolBase))
			return "tool";

		if (source.IsInherited(PlayerBase)) {
#ifdef EXPANSIONMODAI
			if (source.IsInherited(eAIBase))
				return "eai_fist";
#endif
			return "player_fist";
		}

		EntityAI eai;
		if (!Class.CastTo(eai, source))
			return "object";

		Man rootMan = eai.GetHierarchyRootPlayer();
		if (rootMan) {
			string type;
			if (eai.IsHeavyBehaviour())
				type = "_heavy";
			else if (eai.IsTwoHandedBehaviour())
				type = "_two_hands";
			else if (eai.IsOneHandedBehaviour())
				type = "_one_hand";

#ifdef EXPANSIONMODAI
			if (rootMan.IsInherited(eAIBase))
				return "eai_melee_item" + type;
#endif
			return "player_melee_item" + type;
		}

		return "unknown";
	}

	/**
	    \brief Helper to increment value in a specific map and pre-cache labels.
	    \param metricsStore The registry map to update.
	    \param key The normalized source key.
	*/
	protected static void IncMap(map<string, int> metricsStore, string key)
	{
		int value;
		if (metricsStore.Find(key, value)) {
			metricsStore.Set(key, value + 1);
			return;
		}

		metricsStore.Insert(key, 1);
		LabelsFor(key);
	}

}
#endif
