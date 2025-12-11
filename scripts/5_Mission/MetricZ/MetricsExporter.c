/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Background exporter for MetricZ.
    \details Owns scheduling, file I/O and coordination between collectors.
             Entry points: Init() on mission start, Shutdown() on mission finish.
*/
class MetricZ_Exporter
{
	// singleton instance
	protected static ref MetricZ_Exporter s_Instance;

	// Re-entrancy guard: prevents overlapping scrapes.
	protected bool s_Busy;

	/**
	    \brief Initialize MetricZ and schedule first scrape.
	    \details Loads config, initializes storage and queues first Update().
	    \return true if initialized this call, false if already initialized.
	*/
	void MetricZ_Exporter()
	{
		if (s_Instance)
			return;

		if (!MetricZ_Config.IsLoaded())
			MetricZ_Config.Get();

		MetricZ_Storage.Init();

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(
		          Update,
		          MetricZ_Config.Get().settings.init_delay_sec * 1000,
		          false);

		ErrorEx("MetricZ loaded", ErrorExSeverity.INFO);
	}

	/**
	    \brief Access global exporter instance.
	*/
	static MetricZ_Exporter Get()
	{
		if (!s_Instance)
			s_Instance = new MetricZ_Exporter();

		return s_Instance;
	}

	/**
	    \brief Graceful shutdown. Publishes status=0 and stops scheduling.
	    \details Removes scheduled Update(), deletes temp file and overwrites
	             target file with a single status metric.
	*/
	void Shutdown()
	{
		if (!MetricZ_Config.IsLoaded() || MetricZ_Config.Get().http.enabled)
			return;

		// stop future scrapes
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Update);
		DeleteFile(MetricZ_Constants.TEMP_FILE);

		s_Instance = null;
		s_Busy = false;

		ErrorEx("MetricZ scrape shutting down", ErrorExSeverity.INFO);

		MetricZ_FileSink sink = new MetricZ_FileSink();
		if (!sink)
			return;

		sink.SetBuffer(0);
		if (!sink.Begin())
			return;

		MetricZ_Storage.s_Status.Set(0);
		MetricZ_Storage.s_Status.FlushWithHead(sink, MetricZ_Storage.GetExtraLabels());

		sink.End();
	}

	/**
	    \brief Flush all enabled collectors into an open file.
	    \details
	        - Writes world-level metrics from MetricZ_Storage.
	        - Appends per-player, per-entity and event metrics depending on config.
	        - Does not close the file handle.
	    \return true on success, false if fh is invalid.
	*/
	bool Flush()
	{
		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();
		if (!cfg)
			return false;

		MetricZ_SinkBase sink;

		if (cfg.file.enabled && cfg.http.enabled) {
			MetricZ_CompositeSink composite = new MetricZ_CompositeSink();
			if (!composite)
				return false;
			composite.AddSink(new MetricZ_RestSink(), cfg.http.buffer);
			composite.AddSink(new MetricZ_FileSink(), cfg.file.buffer);
			sink = composite;

			ErrorEx("MetricZ: Do not use both exports in production (file.enabled=1, http.enabled=1)", ErrorExSeverity.WARNING);

		} else if (cfg.http.enabled) {
			sink = new MetricZ_RestSink();
			if (!sink)
				return false;
			sink.SetBuffer(cfg.http.buffer)

		} else if (cfg.file.enabled) {
			sink = new MetricZ_FileSink();
			if (!sink)
				return false;
			sink.SetBuffer(cfg.file.buffer)

		} else {
			ErrorEx("MetricZ: No exports enabled in config (file.enabled=0, http.enabled=0)", ErrorExSeverity.WARNING);
			return false;
		}

		if (!sink.Begin())
			return false;

		// world metrics
		MetricZ_Storage.Flush(sink);

		// per-player
		if (!cfg.disabled_metrics.players)
			MetricZ_EntitiesWriter.FlushPlayers(sink);

		// per-infected AI type and mind state
		if (!cfg.disabled_metrics.zombies)
			MetricZ_ZombieStats.Flush(sink);

		// per-animal type
		if (!cfg.disabled_metrics.animals)
			MetricZ_AnimalStats.Flush(sink);

		// per-vehicle
		if (!cfg.disabled_metrics.transports)
			MetricZ_EntitiesWriter.FlushTransport(sink);

		// weapon shots
		if (!cfg.disabled_metrics.weapons)
			MetricZ_WeaponStats.Flush(sink);

		// entity hits
		if (!cfg.disabled_metrics.hits)
			MetricZ_HitStats.Flush(sink);

		// per-territory
		if (!cfg.disabled_metrics.territories)
			MetricZ_EntitiesWriter.FlushTerritory(sink);

		// per-effect-area
		if (!cfg.disabled_metrics.areas)
			MetricZ_EntitiesWriter.FlushEffectAreas(sink);

		// dayz game RPC inputs
		if (!cfg.disabled_metrics.rpc_input)
			MetricZ_RpcStats.Flush(sink);

		// dayz game events
		if (!cfg.disabled_metrics.events)
			MetricZ_EventStats.Flush(sink);

		return sink.End();
	}

	/**
	    \brief Periodic scrape task. Writes temp file and atomically publishes it.
	    \details
	        - Reschedules itself each call using fixed interval (minimizes drift).
	        - Skips execution if a previous scrape is still running (s_Busy).
	        - Calls MetricZ_Storage.Update() to refresh gauges.
	        - Flushes all collectors into METRICS_TEMP and atomically replaces METRICS_FILE.
	*/
	protected void Update()
	{
		// Schedule next tick for minimize drift
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(
		          Update,
		          MetricZ_Config.Get().settings.collect_interval_sec * 1000,
		          false);

		if (s_Busy) {
			MetricZ_Storage.s_ScrapeSkippedTotal.Inc();
			ErrorEx(
			    "MetricZ another scrape already works. You might want to set the scrape interval to twice as much.",
			    ErrorExSeverity.WARNING);

			return;
		}

		s_Busy = true;
		float t0 = g_Game.GetTickTime();

#ifdef DIAG
		ErrorEx("MetricZ start update on " + t0 + "s server tick", ErrorExSeverity.INFO);
#endif

		// refresh world gauges before flush
		MetricZ_Storage.Update();

		// write full snapshot
		if (!Flush()) {
			s_Busy = false;
			return;
		}

		// update labels cache if needed
		MetricZ_PersistentCache.Save();

		float t1 = g_Game.GetTickTime();
		MetricZ_Storage.s_UpdateDurationSec.Set(t1 - t0);

#ifdef DIAG
		ErrorEx("MetricZ stored in " + MetricZ_Storage.s_UpdateDurationSec.Get().ToString() + "s", ErrorExSeverity.INFO);
#endif

		s_Busy = false;
	}
}
#endif
