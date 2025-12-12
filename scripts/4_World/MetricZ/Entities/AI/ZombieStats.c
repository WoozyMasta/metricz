/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Infected live gauges.
    \details
      - tracks per-mind-state counts
      - tracks infected count by canonical zombie type
*/
class MetricZ_ZombieStats
{
	// synthetic mind-state id for dead infected
	static const int MINDSTATE_DEAD = 1000;

	// mind-state -> count
	protected static ref map<int, int> s_StareStorage = new map<int, int>(); //!< State -> current infected count.
	protected static ref map<int, string> s_MindStates; //!< State -> human-readable name. Built lazily.

	// zombie type -> count
	protected static ref map<string, int> s_TypeStorage = new map<string, int>(); //!< Type -> current infected count.
	// zombie type -> cached labels
	protected static ref map<string, string> s_TypeLabels = new map<string, string>();

	// metrics
	protected static ref MetricZ_MetricInt s_MetricMindState = new MetricZ_MetricInt(
	    "infected_mind_state",
	    "Infected count by mind state",
	    MetricZ_MetricType.GAUGE);
	protected static ref MetricZ_MetricInt s_MetricCountByType = new MetricZ_MetricInt(
	    "infected_by_type",
	    "Infected count by zombie type",
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
		s_MindStates.Insert(MINDSTATE_DEAD, "dead");
	}

	/**
	    \brief Apply delta to a mind-state bucket.
	    \details Ignores negative state. Never goes below zero.
	    \param state Mind state id
	    \param delta +/- amount
	*/
	protected static void AddState(int state, int delta)
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
	    \brief Apply delta to a zombie-type bucket.
	    \details Empty type is ignored. Never goes below zero, removes when reaches 0.
	    \param type Canonical zombie type
	    \param delta +/- amount
	*/
	protected static void AddType(string type, int delta)
	{
		type.TrimInPlace();
		if (type == string.Empty || delta == 0)
			return;

		int v;
		if (s_TypeStorage.Find(type, v)) {
			v = v + delta;
			if (v <= 0) {
				s_TypeStorage.Remove(type);
				s_TypeLabels.Remove(type);
			} else
				s_TypeStorage.Set(type, v);
		} else if (delta > 0) {
			s_TypeStorage.Insert(type, delta);

			// build labels once per type
			map<string, string> labels = new map<string, string>();
			labels.Insert("type", type);
			s_TypeLabels.Insert(type, MetricZ_LabelUtils.MakeLabels(labels));
		}
	}

	/**
	    \brief Increment type bucket on spawn.
	    \param zombie Infected instance
	*/
	static void OnSpawn(ZombieBase zombie)
	{
		if (!zombie)
			return;

		string type = zombie.MetricZ_GetLabelTypeName();
		AddType(type, 1);
	}

	/**
	    \brief Decrement buckets on delete.
	    \param zombie Infected instance
	    \param state Last known mind state
	*/
	static void OnDelete(ZombieBase zombie, int state)
	{
		AddState(state, -1);

		if (zombie) {
			string type = zombie.MetricZ_GetLabelTypeName();
			AddType(type, -1);
		}
	}

	/**
	    \brief Move one unit from old to new mind state.
	    \details No-op if unchanged.
	*/
	static void OnStateChange(int oldState, int newState)
	{
		if (oldState == newState)
			return;

		AddState(oldState, -1);
		AddState(newState, 1);
	}

	/**
	    \brief Move from live mind state bucket to "dead".
	    \param lastState Last known live mind state
	*/
	static void OnKilled(int lastState)
	{
		if (lastState >= 0)
			AddState(lastState, -1);

		AddState(MINDSTATE_DEAD, 1);
	}

	/**
	    \brief Emit HELP/TYPE and all state/type samples.
	    \param MetricZ_SinkBase sink instance
	*/
	static void Flush(MetricZ_SinkBase sink)
	{
		if (!sink)
			return;

#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		bool hasStates = (s_StareStorage.Count() > 0);
		bool hasTypes = (s_TypeStorage.Count() > 0);

		if (!hasStates && !hasTypes)
			return;

		// mind states
		if (hasStates) {
			EnsureNames();
			s_MetricMindState.WriteHeaders(sink);

			foreach (int id, int val : s_StareStorage) {
				s_MetricMindState.Set(val);

				string name = "unknown";
				s_MindStates.Find(id, name);

				map<string, string> mindLabels = new map<string, string>();
				mindLabels.Insert("state_id", id.ToString());
				mindLabels.Insert("state", name);

				s_MetricMindState.Flush(sink, MetricZ_LabelUtils.MakeLabels(mindLabels));
			}
		}

		// zombie types
		if (hasTypes) {
			s_MetricCountByType.WriteHeaders(sink);

			foreach (string type, int count : s_TypeStorage) {
				s_MetricCountByType.Set(count);

				string typeLabels = s_TypeLabels.Get(type);
				if (typeLabels == string.Empty) {
					// safety: rebuild labels if cache is empty
					map<string, string> tmp = new map<string, string>();
					tmp.Insert("type", type);
					typeLabels = MetricZ_LabelUtils.MakeLabels(tmp);
					s_TypeLabels.Set(type, typeLabels);
				}

				s_MetricCountByType.Flush(sink, typeLabels);
			}
		}

#ifdef DIAG
		ErrorEx("MetricZ: infected_mind_state / infected_by_type scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}
}
#endif
