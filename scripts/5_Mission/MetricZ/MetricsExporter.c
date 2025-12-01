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

		MetricZ_Config.Load();
		MetricZ_Storage.Init();
		MetricZ_WeaponStats.LoadCache();
		MetricZ_HitStats.LoadCache();

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(
		          Update,
		          MetricZ_Config.s_InitDelayMs,
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
		// stop future scrapes
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Update);
		DeleteFile(MetricZ_Config.METRICS_TEMP);

		s_Instance = null;
		s_Busy = false;

		ErrorEx("MetricZ scrape shutting down", ErrorExSeverity.INFO);

		FileHandle fh = OpenFile(MetricZ_Config.METRICS_FILE, FileMode.WRITE);
		if (!fh)
			return;

		MetricZ_Storage.s_Status.Set(0);
		MetricZ_Storage.s_Status.FlushWithHead(fh, MetricZ_Storage.GetExtraLabels());

		CloseFile(fh);
	}


	/**
	    \brief Flush all enabled collectors into an open file.
	    \details
	        - Writes world-level metrics from MetricZ_Storage.
	        - Appends per-player, per-entity and event metrics depending on config.
	        - Does not close the file handle.
	    \param fh Open file handle to write to (METRICS_TEMP file).
	    \return true on success, false if fh is invalid.
	*/
	bool Flush(FileHandle fh)
	{
		if (!fh)
			return false;

		// world metrics
		MetricZ_Storage.Flush(fh);

		// per-player
		if (!MetricZ_Config.s_DisablePlayerMetrics)
			MetricZ_EntitiesWriter.FlushPlayers(fh);

		// per-infected AI type and mind state
		if (!MetricZ_Config.s_DisableZombieMetrics)
			MetricZ_ZombieStats.Flush(fh);

		// per-animal type
		if (!MetricZ_Config.s_DisableAnimalMetrics)
			MetricZ_AnimalStats.Flush(fh);

		// per-vehicle
		if (!MetricZ_Config.s_DisableTransportMetrics)
			MetricZ_EntitiesWriter.FlushTransport(fh);

		// weapon shots
		if (!MetricZ_Config.s_DisableWeaponMetrics)
			MetricZ_WeaponStats.Flush(fh);

		// entity hits
		if (!MetricZ_Config.s_DisableEntityHitsMetrics)
			MetricZ_HitStats.Flush(fh);

		// per-territory
		if (!MetricZ_Config.s_DisableTerritoryMetrics)
			MetricZ_EntitiesWriter.FlushTerritory(fh);

		// dayz game RPC inputs
		if (!MetricZ_Config.s_DisableRPCMetrics)
			MetricZ_RpcStats.Flush(fh);

		// dayz game events
		if (!MetricZ_Config.s_DisableEventMetrics)
			MetricZ_EventStats.Flush(fh);

		return true;
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
		          MetricZ_Config.s_ScrapeIntervalMs,
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

		FileHandle fh = OpenFile(MetricZ_Config.METRICS_TEMP, FileMode.WRITE);
		if (!fh) {
			s_Busy = false;
			ErrorEx("MetricZ open file " + MetricZ_Config.METRICS_TEMP + " failed");
			return;
		}

		// refresh world gauges before flush
		MetricZ_Storage.Update();

		// write full snapshot into METRICS_TEMP
		if (!Flush(fh)) {
			CloseFile(fh);
			s_Busy = false;
			ErrorEx("MetricZ flush failed", ErrorExSeverity.ERROR);
			return;
		}

		CloseFile(fh);

		// publish new snapshot
		DeleteFile(MetricZ_Config.METRICS_FILE);
		if (!CopyFile(MetricZ_Config.METRICS_TEMP, MetricZ_Config.METRICS_FILE))
			ErrorEx("MetricZ publish failed " + MetricZ_Config.METRICS_TEMP + " -> " + MetricZ_Config.METRICS_FILE);

		DeleteFile(MetricZ_Config.METRICS_TEMP);

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
