/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Suspends metric collection while the backend is unavailable.
    \details Motivation: when the exporter is down or rejecting payloads, the
             previous behaviour kept the full pipeline running - every collect
             interval iterated all collectors, built payloads and issued HTTP
             requests that were known to fail. That burns CPU on the server main
             thread and produces nothing.

             Behaviour: every REST request that exhausts its retries reports a
             failure. After `http.breaker_failures` consecutive failures the
             breaker opens. While open:

             - `MetricZ_Exporter.Update()` skips the whole cycle if the file sink
               is disabled (nothing left to write to).
             - `MetricZ_RestSink` stops emitting HTTP requests if the file sink is
               enabled, so local export keeps working.
             - A single GET against `http.health_path` is issued on an
               exponential backoff with jitter, capped at
               `http.breaker_max_delay_ms`.

             The first successful probe (or any successful regular request)
             closes the breaker and resumes normal collection immediately.

             Set `http.breaker_failures = 0` to disable the breaker entirely and
             restore the previous always-collect behaviour.
*/
class MetricZ_RestCircuitBreaker
{
	protected static bool s_Open; //!< True while collection is suspended
	protected static int s_ConsecutiveFailures; //!< Failed requests since last success
	protected static int s_ProbeAttempt; //!< Probe counter, drives the backoff curve
	protected static int s_SuspendedSince; //!< Timestamp (ms) when the breaker opened
	protected static int s_OpenedTotal; //!< How often the breaker opened (metric)
	protected static int s_SkippedCycles; //!< Collection cycles skipped while open (metric)
	protected static ref MetricZ_CallbackHealth s_Probe; //!< Owning reference to the in-flight probe

	protected static ref MetricZ_MetricInt s_MetricState = new MetricZ_MetricInt(
	    "backend_unavailable",
	    "1 while metric collection is suspended because the backend is unreachable",
	    MetricZ_MetricType.GAUGE);
	protected static ref MetricZ_MetricInt s_MetricOpened = new MetricZ_MetricInt(
	    "backend_outages",
	    "Total number of times collection was suspended due to backend unavailability",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricSkipped = new MetricZ_MetricInt(
	    "scrape_suspended",
	    "Total scrapes skipped because the backend was unavailable",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Whether metric collection is currently suspended.
	    \return bool true if the backend is considered unavailable.
	*/
	static bool IsOpen()
	{
		return s_Open;
	}

	/**
	    \brief Report a request that completed successfully.
	    \details Closes the breaker if it was open and clears the failure streak.
	*/
	static void ReportSuccess()
	{
		s_ConsecutiveFailures = 0;

		if (s_Open)
			Close();
	}

	/**
	    \brief Report a request that failed after exhausting all its retries.
	    \details Opens the breaker once the configured threshold is reached.
	*/
	static void ReportFailure()
	{
		if (!MetricZ_Config.IsLoaded())
			return;

		int threshold = MetricZ_Config.Get().http.breaker_failures;
		if (threshold <= 0)
			return;

		// Already suspended: further failures are expected, the probe drives recovery.
		if (s_Open)
			return;

		s_ConsecutiveFailures++;
		if (s_ConsecutiveFailures < threshold)
			return;

		Open();
	}

	/**
	    \brief Account for a collection cycle that was skipped while suspended.
	*/
	static void ReportSkippedCycle()
	{
		s_SkippedCycles++;
	}

	/**
	    \brief Result handler for a liveness probe.
	    \details Called by `MetricZ_CallbackHealth`. Any 2xx response resumes
	             collection, anything else schedules the next probe.
	    \param ok true if the backend answered successfully.
	*/
	static void OnProbeResult(bool ok)
	{
		s_Probe = null;

		if (!s_Open)
			return;

		if (ok) {
			ReportSuccess();
			return;
		}

		ScheduleProbe();
	}

	/**
	    \brief Reset all state. Called on shutdown.
	*/
	static void Reset()
	{
		if (g_Game)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Probe);

		s_Open = false;
		s_ConsecutiveFailures = 0;
		s_ProbeAttempt = 0;
		s_Probe = null;
	}

	/**
	    \brief Emit breaker metrics to sink.
	    \param sink MetricZ_SinkBase sink instance
	*/
	static void Flush(MetricZ_SinkBase sink)
	{
		if (!sink || !MetricZ_Config.IsLoaded())
			return;

		int state = 0;
		if (s_Open)
			state = 1;

		s_MetricState.Set(state);
		s_MetricState.FlushWithHead(sink);

		s_MetricOpened.Set(s_OpenedTotal);
		s_MetricOpened.FlushWithHead(sink);

		s_MetricSkipped.Set(s_SkippedCycles);
		s_MetricSkipped.FlushWithHead(sink);
	}

	/**
	    \brief Suspend collection and start probing the backend.
	*/
	protected static void Open()
	{
		s_Open = true;
		s_ProbeAttempt = 0;
		s_OpenedTotal++;

		if (g_Game)
			s_SuspendedSince = g_Game.GetTime();

		ErrorEx(
		    string.Format(
		        "MetricZ: backend unavailable after %1 consecutive failed requests, suspending collection until %2 responds",
		        s_ConsecutiveFailures,
		        MetricZ_Config.Get().http.health_path),
		    ErrorExSeverity.WARNING);

		ScheduleProbe();
	}

	/**
	    \brief Resume normal collection.
	*/
	protected static void Close()
	{
		int downtime = 0;
		if (g_Game) {
			downtime = g_Game.GetTime() - s_SuspendedSince;
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Probe);
		}

		s_Open = false;
		s_ProbeAttempt = 0;
		s_Probe = null;

		// Any log line suppressed during the outage is worth seeing now that it ended.
		MetricZ_LogThrottle.FlushAll(ErrorExSeverity.WARNING, "backend recovered");

		ErrorEx(
		    string.Format("MetricZ: backend available again after %1 ms, resuming collection", downtime),
		    ErrorExSeverity.INFO);
	}

	/**
	    \brief Queue the next liveness probe using exponential backoff with jitter.
	*/
	protected static void ScheduleProbe()
	{
		if (!g_Game || !MetricZ_Config.IsLoaded())
			return;

		MetricZ_ConfigDTO_HttpExport cfg = MetricZ_Config.Get().http;

		int backoff = cfg.breaker_delay_ms;
		if (s_ProbeAttempt > 0 && s_ProbeAttempt < 31)
			backoff = cfg.breaker_delay_ms << s_ProbeAttempt;

		if (backoff > cfg.breaker_max_delay_ms || backoff <= 0)
			backoff = cfg.breaker_max_delay_ms;

		// Jitter (+/- 25%) so a fleet of servers does not stampede a recovering backend.
		int delay = (int)Math.Floor(backoff * Math.RandomFloat(0.75, 1.25));

		s_ProbeAttempt++;

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Probe);
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Probe, delay, false);
	}

	/**
	    \brief Issue a single liveness probe against the backend.
	*/
	protected static void Probe()
	{
		if (!s_Open)
			return;

		// A probe is still in flight, do not pile up requests.
		if (s_Probe) {
			ScheduleProbe();
			return;
		}

		MetricZ_RestClient client = MetricZ_RestClient.Get();
		if (!client) {
			ScheduleProbe();
			return;
		}

		s_Probe = new MetricZ_CallbackHealth();
		client.CheckHealth(s_Probe);
	}
}
#endif
