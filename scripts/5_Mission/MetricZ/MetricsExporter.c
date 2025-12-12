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
	protected static ref MetricZ_Exporter s_Instance; // singleton instance

	protected bool s_Busy; // Guard prevents overlapping scrapes
	protected ref map<string, float> m_UpdatesBuffer; // Values buffer by component -> value
	protected ref map<string, string> m_LabelsCache; // Labels cache by component -> label

	protected ref MetricZ_MetricFloat m_UpdateDuration = new MetricZ_MetricFloat(
	    "update_duration_seconds",
	    "Duration of previous MetricZ update, seconds",
	    MetricZ_MetricType.GAUGE);
	protected ref MetricZ_MetricFloat m_ScrapeDuration = new MetricZ_MetricFloat(
	    "scrape_duration_seconds",
	    "Duration of specific scrape components in seconds",
	    MetricZ_MetricType.GAUGE);

	/**
	    \brief Initialize MetricZ and schedule first scrape.
	    \details Loads config, initializes storage and queues first Update().
	    \return true if initialized this call, false if already initialized.
	*/
	void MetricZ_Exporter()
	{
		if (s_Instance)
			return;

		m_UpdatesBuffer = new map<string, float>();
		m_LabelsCache = new map<string, string>();

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

		// write full snapshot
		float t = g_Game.GetTickTime();
		if (!Flush()) {
			s_Busy = false;
			return;
		}
		m_UpdateDuration.Set(g_Game.GetTickTime() - t);

		// update labels cache if needed
		MetricZ_PersistentCache.Save();
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

		// Reset buffer for new cycle
		m_UpdatesBuffer.Clear();

		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();

		// world metrics
		float t = g_Game.GetTickTime();
		MetricZ_Storage.Update();
		MetricZ_Storage.Flush(sink);
		RecordProfile("world", t);

		// per-player
		if (!cfg.disabled_metrics.players) {
			t = g_Game.GetTickTime();
			MetricZ_EntitiesWriter.FlushPlayers(sink);
			RecordProfile("players", t);
		}

		// per-infected AI type and mind state
		if (!cfg.disabled_metrics.zombies) {
			t = g_Game.GetTickTime();
			MetricZ_ZombieStats.Flush(sink);
			RecordProfile("zombies", t);
		}

		// per-animal type
		if (!cfg.disabled_metrics.animals) {
			t = g_Game.GetTickTime();
			MetricZ_AnimalStats.Flush(sink);
			RecordProfile("animals", t);
		}

		// per-vehicle
		if (!cfg.disabled_metrics.transports) {
			t = g_Game.GetTickTime();
			MetricZ_EntitiesWriter.FlushTransport(sink);
			RecordProfile("transports", t);
		}

		// weapon shots
		if (!cfg.disabled_metrics.weapons) {
			t = g_Game.GetTickTime();
			MetricZ_WeaponStats.Flush(sink);
			RecordProfile("weapons", t);
		}

		// entity hits
		if (!cfg.disabled_metrics.hits) {
			t = g_Game.GetTickTime();
			MetricZ_HitStats.Flush(sink);
			RecordProfile("hits", t);
		}

		// per-territory
		if (!cfg.disabled_metrics.territories) {
			t = g_Game.GetTickTime();
			MetricZ_EntitiesWriter.FlushTerritory(sink);
			RecordProfile("territories", t);
		}

		// per-effect-area
		if (!cfg.disabled_metrics.areas) {
			t = g_Game.GetTickTime();
			MetricZ_EntitiesWriter.FlushEffectAreas(sink);
			RecordProfile("areas", t);
		}

		// dayz game RPC inputs
		if (!cfg.disabled_metrics.rpc_input) {
			t = g_Game.GetTickTime();
			MetricZ_RpcStats.Flush(sink);
			RecordProfile("rpc", t);
		}

		// dayz game events
		if (!cfg.disabled_metrics.events) {
			t = g_Game.GetTickTime();
			MetricZ_EventStats.Flush(sink);
			RecordProfile("events", t);
		}

		// http stats
		if (!cfg.disabled_metrics.http && cfg.http.enabled) {
			t = g_Game.GetTickTime();
			MetricZ_HttpStats.Flush(sink);
			RecordProfile("http", t);
		}

		// Write all profiling metrics at the end as a single block
		FlushProfiles(sink);

		// Flush previous update duration
		m_UpdateDuration.FlushWithHead(sink);

		return sink.End();
	}

	/**
	    \brief Records duration into map buffer.
	    \param component Name of the component.
	    \param startTime Tick time when the operation started.
	*/
	protected void RecordProfile(string component, float startTime)
	{
		m_UpdatesBuffer.Set(component, g_Game.GetTickTime() - startTime);
	}

	/**
	    \brief Writes buffered profiling metrics to the sink.
	*/
	protected void FlushProfiles(MetricZ_SinkBase sink)
	{
		if (m_UpdatesBuffer.Count() == 0)
			return;

		m_ScrapeDuration.WriteHeaders(sink);
		foreach (string key, int value : m_UpdatesBuffer) {
			string label;

			if (!m_LabelsCache.Find(key, label)) {
				map<string, string> labelsMap = new map<string, string>();
				labelsMap.Insert("component", key);
				label = MetricZ_LabelUtils.MakeLabels(labelsMap);
				m_LabelsCache.Insert(key, label);
			}

			m_ScrapeDuration.Set(value);
			m_ScrapeDuration.Flush(sink, label);
		}
	}
}
#endif
