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

		ErrorEx("MetricZ: loaded", ErrorExSeverity.INFO);
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
		if (!MetricZ_Config.IsLoaded())
			return;

		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();
		if (cfg.http.enabled)
			return;

		ErrorEx("MetricZ: scrape shutting down", ErrorExSeverity.INFO);

		// unlock
		s_Instance = null;
		s_Busy = false;

		// stop future scrapes
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Update);
		DeleteFile(cfg.file.temp_file_path);

		if (cfg.file.delete_on_shutdown) {
			DeleteFile(cfg.file.prom_file_path);
			return;
		}

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

		if (!MetricZ_Config.IsLoaded())
			return;

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
		ErrorEx("MetricZ: start update on " + t0 + "s server tick", ErrorExSeverity.INFO);
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
		ErrorEx("MetricZ: stored in " + MetricZ_Storage.s_UpdateDurationSec.Get().ToString() + "s", ErrorExSeverity.INFO);
#endif

		s_Busy = false;
	}


	/**
	    \brief Flush all enabled collectors into an open file.
	    \return true on success, false if fh is invalid.
	*/
	protected bool Flush()
	{
		MetricZ_SinkBase sink = MetricZ_Sink.New();
		if (!sink || !sink.Begin())
			return false;

		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();

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

		// http stats
		if (!cfg.disabled_metrics.http && cfg.http.enabled)
			MetricZ_HttpStats.Flush(sink);

		return sink.End();
	}
}
#endif
