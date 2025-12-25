/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Global registry of TerritoryFlag entities.
    \details Keeps track of all TerritoryFlags.
             for metrics collection. Updated on EEInit/EEDelete.
*/
class MetricZ_TerritoryRegistry
{
	protected static ref array<TerritoryFlag> s_Registry = new array<TerritoryFlag>();

	/**
	    \brief Register new TerritoryFlag in the registry.
	    \details Ignores duplicates and null references.
	    \param territory TerritoryFlag instance
	*/
	static void Register(TerritoryFlag territory)
	{
		if (!territory)
			return;

		if (s_Registry.Find(territory) == -1)
			s_Registry.Insert(territory);
	}

	/**
	    \brief Remove TerritoryFlag from the registry.
	    \details Does nothing if not found or null.
	    \param territory TerritoryFlag instance
	*/
	static void Unregister(TerritoryFlag territory)
	{
		if (!territory)
			return;

		int i = s_Registry.Find(territory);
		if (i >= 0)
			s_Registry.Remove(i);
	}

	/**
	    \brief Create a snapshot copy of current registered territories.
	    \details Used to safely iterate without modifying the live registry.
	    \param[out] territories Output array with snapshot of territories
	*/
	static void Snapshot(out array<TerritoryFlag> territories)
	{
		territories = new array<TerritoryFlag>();
		for (int i = 0; i < s_Registry.Count(); ++i)
			territories.Insert(s_Registry[i]);
	}

	/**
	    \brief Returns direct reference to the live registry
	    \warning Do NOT modify this array (Remove/Insert) while iterating!
	*/
	static array<TerritoryFlag> GetList()
	{
		return s_Registry;
	}
}
#endif
