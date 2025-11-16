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
	// Re-entrancy guard: prevents overlapping scrapes.
	protected static bool s_Busy;

	// One-time init flag: ensures Init() is idempotent.
	protected static bool s_Init;

	/**
	    \brief Initialize MetricZ and schedule first scrape.
	    \details Loads config, initializes storage and queues first Update().
	    \return true if initialized this call, false if already initialized.
	*/
	static bool Init()
	{
		if (s_Init)
			return false;

		MetricZ_Config.Load();
		MetricZ_Storage.Init();

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(
		          Update,
		          MetricZ_Config.s_InitDelayMs,
		          false);

		s_Init = true;
		ErrorEx("MetricZ loaded", ErrorExSeverity.INFO);

		return true;
	}

	/**
	    \brief Graceful shutdown. Publishes status=0 and stops scheduling.
	    \details Removes scheduled Update(), deletes temp file and overwrites
	             target file with a single status metric.
	*/
	static void Shutdown()
	{
		if (!s_Init)
			return;

		// stop future scrapes
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Update);
		DeleteFile(MetricZ_Config.TEMP);

		s_Init = false;
		s_Busy = false;

		ErrorEx("MetricZ scrape shutting down", ErrorExSeverity.INFO);

		FileHandle fh = OpenFile(MetricZ_Config.FILE, FileMode.WRITE);
		if (!fh)
			return;

		MetricZ_Storage.s_Status.Set(0);
		MetricZ_Storage.s_Status.FlushWithHead(fh, MetricZ_Storage.GetExtraLabels());

		CloseFile(fh);
	}

	/**
	    \brief Periodic scrape task. Writes temp file and atomically publishes it.
	    \details
	        - Reschedules itself each call (fixed interval, minimal drift).
	        - Skips run if a previous scrape is still running (s_Busy).
	        - Scrapes world metrics and all enabled collectors into TEMP,
	          then atomically publishes to FILE.
	*/
	protected static void Update()
	{
		if (!s_Init)
			return;

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

		FileHandle fh = OpenFile(MetricZ_Config.TEMP, FileMode.WRITE);
		if (!fh) {
			s_Busy = false;
			ErrorEx("MetricZ open file " + MetricZ_Config.TEMP + " failed");
			return;
		}

		// world metrics
		MetricZ_Storage.Update();
		MetricZ_Storage.Flush(fh);

#ifdef DIAG
		ErrorEx("MetricZ base metrics scraped in " + (g_Game.GetTickTime() - t0).ToString() + "s", ErrorExSeverity.INFO);
#endif

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

		// per-territory
		if (!MetricZ_Config.s_DisableTerritoryMetrics)
			MetricZ_EntitiesWriter.FlushTerritory(fh);

		// dayz game RPC inputs
		if (!MetricZ_Config.s_DisableRPCMetrics)
			MetricZ_RpcStats.Flush(fh);

		// dayz game events
		if (!MetricZ_Config.s_DisableEventMetrics)
			MetricZ_EventStats.Flush(fh);

		CloseFile(fh);

		// publish new snapshot
		DeleteFile(MetricZ_Config.FILE);
		if (!CopyFile(MetricZ_Config.TEMP, MetricZ_Config.FILE))
			ErrorEx("MetricZ publish failed " + MetricZ_Config.TEMP + " -> " + MetricZ_Config.FILE);

		DeleteFile(MetricZ_Config.TEMP);

		float t1 = g_Game.GetTickTime();
		MetricZ_Storage.s_UpdateDurationSec.Set(t1 - t0);

#ifdef DIAG
		ErrorEx("MetricZ stored in " + MetricZ_Storage.s_UpdateDurationSec.Get().ToString() + "s", ErrorExSeverity.INFO);
#endif

		s_Busy = false;
	}
}
#endif
