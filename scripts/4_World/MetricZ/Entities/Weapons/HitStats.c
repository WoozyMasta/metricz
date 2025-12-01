/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Damage counters aggregator.
    \details Tracks hits by ammo type and weapon source for players and creatures.
*/
class MetricZ_HitStats
{
	protected static bool s_CacheLoaded;

	// Registries
	protected static ref map<string, int> s_PlayerHit = new map<string, int>();
	protected static ref map<string, int> s_CreatureHit = new map<string, int>();

	// Shared caches for Ammo labels
	protected static ref map<string, string> s_LabelsAmmo = new map<string, string>();

	// Metrics
	protected static ref MetricZ_MetricInt s_MetricPlayerHit = new MetricZ_MetricInt(
	    "player_hit_by",
	    "Count of hits received by players from specific ammo types",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricCreatureHit = new MetricZ_MetricInt(
	    "creature_hit_by",
	    "Count of hits received by creatures (Zombie/Animals/eAI) from specific ammo types",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Load cache of ammo types for label persistency.
	*/
	static void LoadCache()
	{
		if (s_CacheLoaded || MetricZ_Config.s_DisableEntityHitsMetrics)
			return;

		s_CacheLoaded = true;

		array<string> knownAmmo = MetricZ_PersistentCache.GetKeys(MetricZ_CacheKey.AMMO_TYPES);
		if (!knownAmmo)
			return;

		foreach (string ammo : knownAmmo) {
			if (!s_PlayerHit.Contains(ammo)) {
				s_PlayerHit.Insert(ammo, 0);
				LabelsFor(ammo);
			}

			if (!s_CreatureHit.Contains(ammo)) {
				s_CreatureHit.Insert(ammo, 0);
				LabelsFor(ammo);
			}
		}
	}

	/**
	    \brief Register a hit on a Player.
	*/
	static void OnPlayerHit(string ammo)
	{
		if (ammo == string.Empty)
			return;

		MetricZ_PersistentCache.Register(MetricZ_CacheKey.AMMO_TYPES, ammo);
		IncMap(s_PlayerHit, ammo);
	}

	/**
	    \brief Register a hit on a Creature.
	*/
	static void OnCreatureHit(string ammo)
	{
		if (ammo == string.Empty)
			return;

		MetricZ_PersistentCache.Register(MetricZ_CacheKey.AMMO_TYPES, ammo);
		IncMap(s_CreatureHit, ammo);
	}

	/**
	    \brief Emit metrics to file.
	*/
	static void Flush(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif
		if (!fh)
			return;

		if (s_PlayerHit.Count() > 0) {
			s_MetricPlayerHit.WriteHeaders(fh);

			foreach (string aKey, int aVal : s_PlayerHit) {
				s_MetricPlayerHit.Set(aVal);
				s_MetricPlayerHit.Flush(fh, s_LabelsAmmo.Get(aKey));
			}
		}

		if (s_CreatureHit.Count() > 0) {
			s_MetricCreatureHit.WriteHeaders(fh);

			foreach (string caKey, int caVal : s_CreatureHit) {
				s_MetricCreatureHit.Set(caVal);
				s_MetricCreatureHit.Flush(fh, s_LabelsAmmo.Get(caKey));
			}
		}

#ifdef DIAG
		ErrorEx("MetricZ damage_stats scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
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

	/**
	    \brief Get or build cached labels for a ammo type.
	*/
	protected static string LabelsFor(string type)
	{
		string labels;
		if (s_LabelsAmmo && s_LabelsAmmo.Find(type, labels) && labels != string.Empty)
			return labels;

		map<string, string> labelsMap = new map<string, string>();
		labelsMap.Insert("ammo", type);
		labels = MetricZ_LabelUtils.MakeLabels(labelsMap);

		if (!s_LabelsAmmo)
			s_LabelsAmmo = new map<string, string>();

		s_LabelsAmmo.Set(type, labels);

		return labels;
	}
}
#endif
