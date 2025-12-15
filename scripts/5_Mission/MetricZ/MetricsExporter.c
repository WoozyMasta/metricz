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

	protected int m_FlushStep; //!< Flush State Machine - step number
	protected float m_FlushStartTime; //!< Flush State Machine - start time
	protected float m_BeginDuration; //!< Temporary storage for current cycle timing
	protected bool s_Busy; //!< Guard prevents overlapping scrapes
	protected ref MetricZ_SinkBase m_ActiveSink; //!< Active sink used in Flush
	protected ref map<string, float> m_UpdatesBuffer; //!< Values buffer by component -> value
	protected ref map<string, string> m_LabelsCache; //!< Labels cache by component -> label

	protected ref MetricZ_MetricFloat m_UpdateDuration = new MetricZ_MetricFloat(
	    "update_duration_seconds",
	    "Duration of previous MetricZ update, seconds",
	    MetricZ_MetricType.GAUGE);
	protected ref MetricZ_MetricFloat m_SinkBeginDuration = new MetricZ_MetricFloat(
	    "sink_begin_duration_seconds",
	    "Time spent initializing the metric sink in the previous cycle (e.g. file open I/O or buffer allocation)",
	    MetricZ_MetricType.GAUGE);
	protected ref MetricZ_MetricFloat m_SinkEndDuration = new MetricZ_MetricFloat(
	    "sink_end_duration_seconds",
	    "Time spent finalizing the export in the previous cycle (e.g. file close/atomic swap or HTTP transmission)",
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

		ErrorEx("MetricZ: scrape shutting down", ErrorExSeverity.INFO);
		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();

		m_ActiveSink = null;
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(ProcessFlushStep);

		if (cfg.http.enabled)
			return;

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
			ErrorEx("MetricZ: skip scrape, previous cycle still running", ErrorExSeverity.WARNING);
			return;
		}

		s_Busy = true;
		m_FlushStep = 0;
		m_FlushStartTime = g_Game.GetTickTime();
		m_ActiveSink = MetricZ_Sink.New();
		m_UpdatesBuffer.Clear();

		if (!m_ActiveSink || !m_ActiveSink.Begin()) {
			s_Busy = false;
			m_ActiveSink = null;

			ErrorEx("MetricZ: failed to open sink", ErrorExSeverity.ERROR);
			return;
		}

		// store for later
		m_BeginDuration = g_Game.GetTickTime() - m_FlushStartTime;

		// enter to Flush State Machine
		ProcessFlushStep();
	}

	/**
	    \brief Executes ONE block of metrics and schedules the next block
	*/
	protected void ProcessFlushStep()
	{
		if (!m_ActiveSink) {
			s_Busy = false;
			return;
		}

		float t = g_Game.GetTickTime();
		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();

		// Flush State Machine
		switch (m_FlushStep) {
		case 0: // world metrics
			MetricZ_Storage.Update();
			MetricZ_Storage.Flush(m_ActiveSink);
			RecordProfile("world", t);
			break;

		case 1: // per-player
			if (!cfg.disabled_metrics.players) {
				MetricZ_EntitiesWriter.FlushPlayers(m_ActiveSink);
				RecordProfile("players", t);
			}
			break;

		case 2: // per-infected AI type and mind state
			if (!cfg.disabled_metrics.zombies) {
				MetricZ_ZombieStats.Flush(m_ActiveSink);
				RecordProfile("zombies", t);
			}
			break;

		case 3: // per-animal type
			if (!cfg.disabled_metrics.animals) {
				MetricZ_AnimalStats.Flush(m_ActiveSink);
				RecordProfile("animals", t);
			}
			break;

		case 4: // per-vehicle
			if (!cfg.disabled_metrics.transports) {
				MetricZ_EntitiesWriter.FlushTransport(m_ActiveSink);
				RecordProfile("transports", t);
			}
			break;

		case 5: // weapon shots
			if (!cfg.disabled_metrics.weapons) {
				MetricZ_WeaponStats.Flush(m_ActiveSink);
				RecordProfile("weapons", t);
			}
			break;

		case 6: // entity hits
			if (!cfg.disabled_metrics.hits) {
				MetricZ_HitStats.Flush(m_ActiveSink);
				RecordProfile("hits", t);
			}
			break;

		case 7: // per-territory
			if (!cfg.disabled_metrics.territories) {
				MetricZ_EntitiesWriter.FlushTerritory(m_ActiveSink);
				RecordProfile("territories", t);
			}
			break;

		case 8: // per-effect-area
			if (!cfg.disabled_metrics.areas) {
				MetricZ_EntitiesWriter.FlushEffectAreas(m_ActiveSink);
				RecordProfile("areas", t);
			}
			break;

		case 9: // dayz game RPC inputs
			if (!cfg.disabled_metrics.rpc_input) {
				MetricZ_RpcStats.Flush(m_ActiveSink);
				RecordProfile("rpc", t);
			}
			break;

		case 10: // dayz game events
			if (!cfg.disabled_metrics.events) {
				MetricZ_EventStats.Flush(m_ActiveSink);
				RecordProfile("events", t);
			}
			break;

		case 11: // http stats
			if (!cfg.disabled_metrics.http && cfg.http.enabled) {
				MetricZ_HttpStats.Flush(m_ActiveSink);
				RecordProfile("http", t);
			}
			break;

		case 12: // commit and close
			FinishFlush();
			return;
		}

		// schedule next step for the next server frame
		m_FlushStep++;
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Call(ProcessFlushStep);
	}

	/**
	    \brief Finalizes the scrape cycle.
	*/
	protected void FinishFlush()
	{
		// internal profiling metrics
		FlushProfiles(m_ActiveSink);

		// write durations from previous cycle
		m_UpdateDuration.FlushWithHead(m_ActiveSink);
		m_SinkBeginDuration.FlushWithHead(m_ActiveSink);
		m_SinkEndDuration.FlushWithHead(m_ActiveSink);

		// commit and close
		float t = g_Game.GetTickTime();
		m_ActiveSink.End();
		m_ActiveSink = null;

		// update metrics for next cycle
		m_SinkBeginDuration.Set(m_BeginDuration);
		m_SinkEndDuration.Set(g_Game.GetTickTime() - t);
		m_UpdateDuration.Set(g_Game.GetTickTime() - m_FlushStartTime);

		// update labels cache if needed
		MetricZ_PersistentCache.Save();

		s_Busy = false;
	}

	/**
	    \brief Records duration into map buffer.
	    \param component Name of the component.
	    \param startTime Tick time when the operation started.
	*/
	protected void RecordProfile(string component, float startTime)
	{
		float t = g_Game.GetTickTime() - startTime;
		m_UpdatesBuffer.Set(component, t);

#ifdef DIAG
		ErrorEx(
		    "MetricZ: Flush." + component + " in frame " + m_FlushStep.ToString() + " took: " + (t * 1000).ToString() + "ms",
		    ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Writes buffered profiling metrics to the sink.
	*/
	protected void FlushProfiles(MetricZ_SinkBase sink)
	{
		if (m_UpdatesBuffer.Count() == 0)
			return;

		m_ScrapeDuration.WriteHeaders(sink);
		foreach (string key, float value : m_UpdatesBuffer) {
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
