/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Global registry of EffectArea entities.
    \details Keeps track of all EffectAreas.
             for metrics collection. Updated on EEInit/EEDelete.
*/
class MetricZ_EffectAreaRegistry
{
	protected static ref array<EffectArea> s_Registry = new array<EffectArea>();

	/**
	    \brief Register new EffectArea in the registry.
	    \details Ignores duplicates and null references.
	    \param EffectArea EffectArea instance
	*/
	static void Register(EffectArea area)
	{
		if (!area)
			return;

		if (s_Registry.Find(area) == -1)
			s_Registry.Insert(area);
	}

	/**
	    \brief Remove EffectArea from the registry.
	    \details Does nothing if not found or null.
	    \param EffectArea EffectArea instance
	*/
	static void Unregister(EffectArea area)
	{
		if (!area)
			return;

		int i = s_Registry.Find(area);
		if (i >= 0)
			s_Registry.Remove(i);
	}

	/**
	    \brief Create a snapshot copy of current registered areas.
	    \details Used to safely iterate without modifying the live registry.
	    \param[out] areas Output array with snapshot of areas
	*/
	static void Snapshot(out array<EffectArea> areas)
	{
		areas = new array<EffectArea>();
		for (int i = 0; i < s_Registry.Count(); ++i)
			areas.Insert(s_Registry[i]);
	}

	/**
	    \brief Returns direct reference to the live registry
	    \warning Do NOT modify this array (Remove/Insert) while iterating!
	*/
	static array<EffectArea> GetList()
	{
		return s_Registry;
	}
}
#endif
