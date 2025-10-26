/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Helpers for ordered entity metrics output.
*/
class MetricZ_EntitiesWriter
{
	/**
	    \brief Flush all player metrics in interleaved order.
	    \details For each metric index: write HELP/TYPE once, then values for all players.
	    \param fh Open file handle
	*/
	static void FlushPlayers(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		if (!fh)
			return;

		array<ref MetricZ_PlayerMetrics> pms = new array<ref MetricZ_PlayerMetrics>();

		array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);

		foreach (Man man : players) {
			PlayerBase player;
			if (!Class.CastTo(player, man))
				continue;

			MetricZ_PlayerMetrics pm = player.MetricZ_GetMetrics();
			if (!pm)
				continue;

			pm.Update();
			pms.Insert(pm);
		}

		if (pms.Count() == 0)
			return;

		int metricsCount = pms[0].Count();
		for (int i = 0; i < metricsCount; i++) {
			pms[0].WriteHeaderAt(fh, i);

			foreach (MetricZ_PlayerMetrics pmCurrent : pms)
				pmCurrent.FlushAt(fh, i);
		}

#ifdef DIAG
		ErrorEx("MetricZ player_* scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Flush all transport metrics in interleaved order.
	    \details For each metric index: write HELP/TYPE once, then values for all transport.
	    \param fh Open file handle
	*/
	static void FlushTransport(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		if (!fh)
			return;

		array<Transport> list;
		MetricZ_TransportRegistry.Snapshot(list);
		if (!list || list.Count() == 0)
			return;

		array<ref MetricZ_TransportMetrics> tms = new array<ref MetricZ_TransportMetrics>();
		foreach (Transport transport : list) {
			if (!transport)
				continue;

			MetricZ_TransportMetrics tm;

			CarScript car;
			if (Class.CastTo(car, transport))
				tm = car.MetricZ_GetMetrics();

			BoatScript boat;
			if (!tm && Class.CastTo(boat, transport))
				tm = boat.MetricZ_GetMetrics();

			HelicopterScript heli;
			if (!tm && Class.CastTo(heli, transport))
				tm = heli.MetricZ_GetMetrics();

			if (!tm)
				continue;

			tm.Update();
			tms.Insert(tm);
		}

		if (tms.Count() == 0)
			return;

		int metricsCount = tms[0].Count();
		for (int i = 0; i < metricsCount; i++) {
			tms[0].WriteHeaderAt(fh, i);

			foreach (MetricZ_TransportMetrics tmCurrent : tms)
				tmCurrent.FlushAt(fh, i);
		}

#ifdef DIAG
		ErrorEx("MetricZ transport_* scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}

	static void FlushTerritory(FileHandle fh)
	{
#ifdef DIAG
		float t0 = g_Game.GetTickTime();
#endif

		if (!fh)
			return;

		array<TerritoryFlag> list;
		MetricZ_TerritoryRegistry.Snapshot(list);
		if (!list || list.Count() == 0)
			return;

		array<ref MetricZ_TerritoryMetrics> fms = new array<ref MetricZ_TerritoryMetrics>();
		foreach (TerritoryFlag territory : list) {
			if (!territory)
				continue;

			MetricZ_TerritoryMetrics fm = territory.MetricZ_GetMetrics();
			if (!fm)
				continue;

			fm.Update();
			fms.Insert(fm);
		}

		if (fms.Count() == 0)
			return;

		int n = fms[0].Count();
		for (int i = 0; i < n; i++) {
			fms[0].WriteHeaderAt(fh, i);

			foreach (MetricZ_TerritoryMetrics fmCurrent : fms)
				fmCurrent.FlushAt(fh, i);
		}

#ifdef DIAG
		ErrorEx("MetricZ territory_* scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Flush weapon stats as a grouped family.
	*/
	static void FlushWeapons(FileHandle fh)
	{
		if (!fh)
			return;

		MetricZ_WeaponStats.Flush(fh);
	}
}
#endif
