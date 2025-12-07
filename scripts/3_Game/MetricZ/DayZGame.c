/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    Server-side instrumentation for RPC and Event metrics.
    Counts incoming RPCs by id and all events by EventType.
*/
modded class DayZGame
{
	// number of calls to UpdatePathgraphRegionByObject() on server
	static int s_MetricZ_PathGraphUpdates;

	/**
	    \brief Pump frame monitor each frame.
	    \param timeslice Delta time of the last frame in seconds.
	*/
	override void OnPostUpdate(bool doSim, float timeslice)
	{
		if (doSim)
			MetricZ_FrameMonitor.OnUpdate(timeslice);

		super.OnPostUpdate(doSim, timeslice);
	}

	/**
	    \brief Hook incoming RPCs to count them.
	    \details Increments per-id counter, then calls base.
	*/
	override void OnRPC(PlayerIdentity sender, Object target, int rpc_type, ParamsReadContext ctx)
	{
		// count all input RPC calls
		if (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disableRPCMetrics)
			MetricZ_RpcStats.Inc(rpc_type);

		super.OnRPC(sender, target, rpc_type, ctx);
	}

	/**
	    \brief Hook engine events to count occurrences.
	    \details Increments per-EventType counter, then calls base.
	*/
	override void OnEvent(EventType eventTypeId, Param params)
	{
		// count all events on server
		if (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disableEventMetrics)
			MetricZ_EventStats.Inc(eventTypeId);

		super.OnEvent(eventTypeId, params);
	}

	/**
	    \brief Hook CGame event for calculate path graph updates by placing objects.
	    \details Increments static counter for reuse in MetricZ_Storage.
	*/
	override void UpdatePathgraphRegionByObject(Object object)
	{
		if (object)
			s_MetricZ_PathGraphUpdates++;

		super.UpdatePathgraphRegionByObject(object);
	}
}
#endif
