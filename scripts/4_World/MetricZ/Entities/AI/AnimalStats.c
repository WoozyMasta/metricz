/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Animals-by-type gauges.
    \details Tracks live animals count per canonical type. Updated on EEInit/EEDelete.
*/
class MetricZ_AnimalStats
{
	protected static ref map<string, int> s_CountByType = new map<string, int>(); // animal -> count
	protected static ref map<string, string> s_LabelsByType = new map<string, string>(); // animal -> cached labels

	protected static ref MetricZ_MetricInt s_MetricCountByType = new MetricZ_MetricInt(
	    "animals_by_type",
	    "Animals in world grouped by canonical type",
	    MetricZ_MetricType.GAUGE);

	/**
	    \brief Increment per-type count for spawned animal.
	*/
	static void OnSpawn(AnimalBase animal)
	{
		if (!animal)
			return;

		string type = animal.MetricZ_GetAnimalName();

		int v;
		if (s_CountByType.Find(type, v))
			s_CountByType.Set(type, v + 1);
		else {
			s_CountByType.Insert(type, 1);

			// build labels once per type
			map<string, string> labels = new map<string, string>();
			labels.Insert("animal", type);
			s_LabelsByType.Insert(type, MetricZ_LabelUtils.MakeLabels(labels));
		}
	}

	/**
	    \brief Decrement per-type count for deleted animal.
	*/
	static void OnDelete(AnimalBase animal)
	{
		if (!animal)
			return;

		string type = animal.MetricZ_GetAnimalName();

		int v;
		if (!s_CountByType.Find(type, v))
			return;

		v = v - 1;
		if (v <= 0) {
			s_CountByType.Remove(type);
			s_LabelsByType.Remove(type);
		} else
			s_CountByType.Set(type, v);
	}

	/**
	    \brief Emit HELP/TYPE and all animals_by_type.
	    \param fh Open file handle.
	*/
	static void Flush(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		if (!fh)
			return;

		if (s_CountByType.Count() == 0)
			return;

		s_MetricCountByType.WriteHeaders(fh);

		foreach (string key, int val : s_CountByType) {
			s_MetricCountByType.Set(val);

			string labels = s_LabelsByType.Get(key);
			if (labels == string.Empty) {
				// safety: rebuild labels if cache was empty
				map<string, string> tmp = new map<string, string>();
				tmp.Insert("animal", key);
				labels = MetricZ_LabelUtils.MakeLabels(tmp);
				s_LabelsByType.Set(key, labels);
			}

			s_MetricCountByType.Flush(fh, labels);
		}

#ifdef DIAG
		ErrorEx("MetricZ animals_by_type scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}
}
#endif
