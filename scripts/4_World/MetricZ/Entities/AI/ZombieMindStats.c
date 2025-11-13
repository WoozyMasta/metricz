/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Infected mind-state live gauges.
    \details Tracks per-state counts and flushes as labeled gauge series.
*/
class MetricZ_ZombieMindStats
{
	protected static ref map<int, int> s_StareStorage = new map<int, int>(); //!< State -> current infected count.
	protected static ref map<int, string> s_MindStates; //!< State -> human-readable name. Built lazily.

	protected static ref MetricZ_MetricInt s_Gauge = new MetricZ_MetricInt(
	    "infected_mind_state",
	    "Infected count by mind state",
	    MetricZ_MetricType.GAUGE);

	/**
	    \brief Fill name map once.
	    \details No-op on subsequent calls.
	*/
	protected static void EnsureNames()
	{
		if (s_MindStates)
			return;

		s_MindStates = new map<int, string>();
		s_MindStates.Insert(DayZInfectedConstants.MINDSTATE_CALM, "calm");
		s_MindStates.Insert(DayZInfectedConstants.MINDSTATE_DISTURBED, "disturbed");
		s_MindStates.Insert(DayZInfectedConstants.MINDSTATE_ALERTED, "alerted");
		s_MindStates.Insert(DayZInfectedConstants.MINDSTATE_CHASE, "chase");
		s_MindStates.Insert(DayZInfectedConstants.MINDSTATE_FIGHT, "fight");
	}

	/**
	    \brief Apply delta to a state bucket.
	    \details Ignores negative state. Never goes below zero.
	    \param state Mind state id
	    \param delta +/- amount
	*/
	protected static void Add(int state, int delta)
	{
		if (state < 0)
			return;

		int v;
		if (s_StareStorage.Find(state, v))
			s_StareStorage.Set(state, Math.Max(v + delta, 0));
		else if (delta > 0)
			s_StareStorage.Insert(state, delta);
	}

	/**
	    \brief Increment bucket on spawn.
	    \param state Initial mind state
	*/
	static void OnSpawn(int state)
	{
		Add(state, 1);
	}

	/**
	    \brief Decrement bucket on delete.
	    \param state Last known mind state
	*/
	static void OnDelete(int state)
	{
		Add(state, -1);
	}

	/**
	    \brief Move one unit from old to new state.
	    \details No-op if unchanged.
	*/
	static void OnStateChange(int oldState, int newState)
	{
		if (oldState == newState)
			return;

		Add(oldState, -1);
		Add(newState, 1);
	}

	/**
	    \brief Emit HELP/TYPE and all state samples.
	    \param fh Open file handle
	*/
	static void Flush(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		if (!fh || s_StareStorage.Count() == 0)
			return;

		EnsureNames();
		s_Gauge.WriteHeaders(fh);

		foreach (int id, int val : s_StareStorage) {
			s_Gauge.Set(val);

			string name = "unknown";
			s_MindStates.Find(id, name);

			map<string, string> labels = new map<string, string>();
			labels.Insert("id", id.ToString());
			labels.Insert("state", name);

			s_Gauge.Flush(fh, MetricZ_LabelUtils.MakeLabels(labels));
		}

#ifdef DIAG
		ErrorEx("MetricZ infected_mind_state scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}
}
#endif
