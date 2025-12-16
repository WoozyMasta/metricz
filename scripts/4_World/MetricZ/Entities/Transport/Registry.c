/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Global registry of active Transport entities.
    \details Keeps track of all spawned vehicles (CarScript, BoatScript, HelicopterScript, etc.)
             for metrics collection. Updated on EEInit/EEDelete.
*/
class MetricZ_TransportRegistry
{
	protected static ref array<Transport> s_Registry = new array<Transport>();

	/**
	    \brief Register new transport in the registry.
	    \details Ignores duplicates and null references.
	    \param transport Transport instance
	*/
	static void Register(Transport transport)
	{
		if (!transport)
			return;

		if (s_Registry.Find(transport) == -1)
			s_Registry.Insert(transport);
	}

	/**
	    \brief Remove transport from the registry.
	    \details Does nothing if not found or null.
	    \param transport Transport instance
	*/
	static void Unregister(Transport transport)
	{
		if (!transport)
			return;

		int i = s_Registry.Find(transport);
		if (i >= 0)
			s_Registry.Remove(i);
	}

	/**
	    \brief Create a snapshot copy of current registered transports.
	    \details Used to safely iterate without modifying the live registry.
	    \param[out] transport Output array with snapshot of transport objects
	*/
	static void Snapshot(out array<Transport> transport)
	{
		transport = new array<Transport>();
		for (int i = 0; i < s_Registry.Count(); i++)
			transport.Insert(s_Registry[i]);
	}

	/**
	    \brief Returns direct reference to the live registry
	    \warning Do NOT modify this array (Remove/Insert) while iterating!
	*/
	static array<Transport> GetList()
	{
		return s_Registry;
	}
}
#endif
