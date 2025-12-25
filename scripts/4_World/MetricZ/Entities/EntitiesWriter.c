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
	// Static buffers for Object Pooling
	static ref array<Man> s_PlayersListBuffer = new array<Man>();
	static ref array<ref MetricZ_PlayerMetrics> s_PlayerMetricsBuffer = new array<ref MetricZ_PlayerMetrics>();
	static ref array<ref MetricZ_TransportMetrics> s_TransportMetricsBuffer = new array<ref MetricZ_TransportMetrics>();
	static ref array<ref MetricZ_TerritoryMetrics> s_TerritoryMetricsBuffer = new array<ref MetricZ_TerritoryMetrics>();
	static ref array<ref MetricZ_EffectAreaMetrics> s_AreaMetricsBuffer = new array<ref MetricZ_EffectAreaMetrics>();

	/**
	    \brief Flush all player metrics in interleaved order.
	    \details For each metric index: write HELP/TYPE once, then values for all players.
	    \param MetricZ_SinkBase sink instance
	*/
	static void FlushPlayers(MetricZ_SinkBase sink)
	{
		if (!sink)
			return;

		s_PlayerMetricsBuffer.Clear();
		s_PlayersListBuffer.Clear();
		g_Game.GetPlayers(s_PlayersListBuffer);

		foreach (Man man : s_PlayersListBuffer) {
			PlayerBase player;
			if (!Class.CastTo(player, man))
				continue;

			MetricZ_PlayerMetrics pm = player.MetricZ_GetMetrics();
			if (!pm)
				continue;

			pm.Update();
			s_PlayerMetricsBuffer.Insert(pm);
		}

		if (s_PlayerMetricsBuffer.Count() == 0)
			return;

		int metricsCount = s_PlayerMetricsBuffer[0].Count();
		for (int i = 0; i < metricsCount; ++i) {
			s_PlayerMetricsBuffer[0].WriteHeaderAt(sink, i);

			foreach (MetricZ_PlayerMetrics pmCurrent : s_PlayerMetricsBuffer)
				pmCurrent.GetMetricDirect(i).Flush(sink);
		}
	}

	/**
	    \brief Flush all transport metrics in interleaved order.
	    \details For each metric index: write HELP/TYPE once, then values for all transport.
	    \param MetricZ_SinkBase sink instance
	*/
	static void FlushTransport(MetricZ_SinkBase sink)
	{
		if (!sink)
			return;

		array<Transport> list = MetricZ_TransportRegistry.GetList();
		if (!list || list.Count() == 0)
			return;

		s_TransportMetricsBuffer.Clear();
		foreach (Transport transport : list) {
			if (!transport)
				continue;

			MetricZ_TransportMetrics tm;

			CarScript car = CarScript.Cast(transport);
			if (car)
				tm = car.MetricZ_GetMetrics();
			else {
				BoatScript boat = BoatScript.Cast(transport);
				if (boat)
					tm = boat.MetricZ_GetMetrics();
				else {
					HelicopterScript heli = HelicopterScript.Cast(transport);
					if (heli)
						tm = heli.MetricZ_GetMetrics();
#ifdef EXPANSIONMODVEHICLE
					else {
						ExpansionVehicleBase expansionVeh = ExpansionVehicleBase.Cast(transport);
						if (expansionVeh)
							tm = expansionVeh.MetricZ_GetMetrics();
					}
#endif
				}
			}

			if (!tm)
				continue;

			tm.Update();
			s_TransportMetricsBuffer.Insert(tm);
		}

		if (s_TransportMetricsBuffer.Count() == 0)
			return;

		int metricsCount = s_TransportMetricsBuffer[0].Count();
		for (int i = 0; i < metricsCount; ++i) {
			s_TransportMetricsBuffer[0].WriteHeaderAt(sink, i);

			foreach (MetricZ_TransportMetrics tmCurrent : s_TransportMetricsBuffer)
				tmCurrent.GetMetricDirect(i).Flush(sink);
		}
	}

	/**
	    \brief Flush all territory metrics in interleaved order.
	    \details For each metric index: write HELP/TYPE once, then values for all territory.
	    \param MetricZ_SinkBase sink instance
	*/
	static void FlushTerritory(MetricZ_SinkBase sink)
	{
		if (!sink)
			return;

		array<TerritoryFlag> list = MetricZ_TerritoryRegistry.GetList();
		if (!list || list.Count() == 0)
			return;

		s_TerritoryMetricsBuffer.Clear();
		foreach (TerritoryFlag territory : list) {
			if (!territory)
				continue;

			MetricZ_TerritoryMetrics fm = territory.MetricZ_GetMetrics();
			if (!fm)
				continue;

			fm.Update();
			s_TerritoryMetricsBuffer.Insert(fm);
		}

		if (s_TerritoryMetricsBuffer.Count() == 0)
			return;

		int n = s_TerritoryMetricsBuffer[0].Count();
		for (int i = 0; i < n; ++i) {
			s_TerritoryMetricsBuffer[0].WriteHeaderAt(sink, i);

			foreach (MetricZ_TerritoryMetrics fmCurrent : s_TerritoryMetricsBuffer)
				fmCurrent.GetMetricDirect(i).Flush(sink);
		}
	}

	/**
	    \brief Flush weapon stats as a grouped family.
	    \param MetricZ_SinkBase sink instance
	*/
	static void FlushWeapons(MetricZ_SinkBase sink)
	{
		if (!sink)
			return;

		MetricZ_WeaponStats.Flush(sink);
	}

	/**
	    \brief Flush all EffectArea metrics in interleaved order.
	    \details For each metric index: write HELP/TYPE once, then values for all EffectArea.
	    \param MetricZ_SinkBase sink instance
	*/
	static void FlushEffectAreas(MetricZ_SinkBase sink)
	{
		if (!sink)
			return;

		array<EffectArea> list = MetricZ_EffectAreaRegistry.GetList();
		if (!list || list.Count() == 0)
			return;

		s_AreaMetricsBuffer.Clear();
		foreach (EffectArea area : list) {
			if (!area)
				continue;

			MetricZ_EffectAreaMetrics am = area.MetricZ_GetMetrics();
			if (!am)
				continue;

			am.Update();
			s_AreaMetricsBuffer.Insert(am);
		}

		if (s_AreaMetricsBuffer.Count() == 0)
			return;

		int n = s_AreaMetricsBuffer[0].Count();
		for (int i = 0; i < n; ++i) {
			s_AreaMetricsBuffer[0].WriteHeaderAt(sink, i);

			foreach (MetricZ_EffectAreaMetrics amCurrent : s_AreaMetricsBuffer)
				amCurrent.GetMetricDirect(i).Flush(sink);
		}
	}
}
#endif
