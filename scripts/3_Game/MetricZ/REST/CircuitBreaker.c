/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
//! States of the REST circuit breaker
enum MetricZ_EBreakerState {
	CLOSED, //!< Normal operation, scrapes run and are delivered
	OPEN, //!< Backend unavailable, collection suspended, health is probed
	HALF_OPEN //!< Probe succeeded, one trial scrape must verify the ingest path
}

/**
    \brief Suspends metric collection while the backend is unavailable.
    \details Motivation: when the exporter is down or rejecting payloads, the
             previous behaviour kept the full pipeline running - every collect
             interval iterated all collectors, built payloads and issued HTTP
             requests that were known to fail.

             The breaker tracks results at the scrape level, not per request:
             a scrape counts as failed when any chunk upload or the final
             commit exhausts its retries; it counts as successful only when
             the backend acknowledged the commit (or the single non-transacted
             POST). A successful chunk upload alone never resets the streak.

             State machine:

             - CLOSED: after `http.breaker_failures` consecutive failed
               scrapes the breaker opens.
             - OPEN: no REST requests except a GET against `http.health_path`
               on an exponential backoff with jitter, hard-capped at
               `MetricZ_Constants.BREAKER_MAX_DELAY_MS`. A successful probe
               moves to HALF_OPEN.
             - HALF_OPEN: exactly one trial scrape is allowed through. If its
               commit succeeds the breaker closes; if it fails (or never
               resolves within the trial deadline) the breaker reopens with a
               grown backoff.

             Every scrape draws a ticket (`BeginScrape`). Each state
             transition raises a barrier that invalidates all previously
             issued tickets, so stale callbacks from before the transition
             can neither close nor trip the breaker.

             Set `http.breaker_failures = 0` to disable the breaker entirely.
*/
class MetricZ_RestCircuitBreaker
{
	protected static MetricZ_EBreakerState s_State = MetricZ_EBreakerState.CLOSED; //!< Current breaker state
	protected static int s_TicketCounter; //!< Last issued scrape ticket
	protected static int s_TicketBarrier; //!< Tickets issued before the last state transition are <= barrier and invalid
	protected static int s_ActiveTicket = -1; //!< Ticket of the currently running scrape, -1 if none
	protected static int s_LastFailedTicket = -1; //!< Dedup: one scrape counts at most one failure
	protected static int s_ConsecutiveFailures; //!< Failed scrapes since the last success (CLOSED only)
	protected static int s_ProbeAttempt; //!< Probe counter, drives the backoff curve
	protected static int s_SuspendedSince; //!< Timestamp (ms) when the outage began
	protected static bool s_TrialIssued; //!< HALF_OPEN: the trial ticket has been handed out
	protected static int s_TrialDeadline; //!< HALF_OPEN: when an unresolved trial counts as failed
	protected static int s_OpenedTotal; //!< closed -> open transitions (metric)
	protected static int s_SkippedTotal; //!< Scrapes skipped while suspended (metric)
	protected static int s_SuspendedMsTotal; //!< Cumulative suspension time in ms (metric)
	protected static ref MetricZ_CallbackHealth s_Probe; //!< Owning reference to the in-flight probe

	protected static ref MetricZ_MetricInt s_MetricOpened = new MetricZ_MetricInt(
	    "http_circuit_breaker_opened",
	    "Total number of times the circuit breaker opened because the backend was unavailable",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricSkipped = new MetricZ_MetricInt(
	    "http_circuit_breaker_skipped_scrapes",
	    "Total scrapes skipped while collection was suspended",
	    MetricZ_MetricType.COUNTER);
	protected static ref MetricZ_MetricInt s_MetricSuspendedMs = new MetricZ_MetricInt(
	    "http_circuit_breaker_suspended_ms",
	    "Total time in milliseconds metric collection spent suspended",
	    MetricZ_MetricType.COUNTER);

	/**
	    \brief Draw a ticket for a new scrape.
	    \details CLOSED: always issues. HALF_OPEN: issues exactly one trial
	             ticket; while the trial is unresolved further scrapes are
	             refused, and once the trial deadline passes the trial counts
	             as failed and the breaker reopens. OPEN: always refuses.
	    \return int Ticket to pass to ReportScrapeSuccess/Failure, or -1 if
	                no REST payload may be sent this cycle.
	*/
	static int BeginScrape()
	{
		if (s_State == MetricZ_EBreakerState.CLOSED) {
			s_TicketCounter++;
			s_ActiveTicket = s_TicketCounter;
			return s_ActiveTicket;
		}

		if (s_State == MetricZ_EBreakerState.HALF_OPEN) {
			int now = 0;
			if (g_Game)
				now = g_Game.GetTime();

			if (!s_TrialIssued) {
				s_TrialIssued = true;
				s_TrialDeadline = now + TrialTimeout();
				s_TicketCounter++;
				s_ActiveTicket = s_TicketCounter;
				return s_ActiveTicket;
			}

			// The trial scrape never resolved (sink failed to start, client
			// could not be created, callback lost): treat as failed recovery.
			if (now > s_TrialDeadline)
				Open();
		}

		s_ActiveTicket = -1;
		return -1;
	}

	/**
	    \brief Ticket of the scrape currently running.
	    \return int Ticket issued by the last BeginScrape(), -1 if refused.
	*/
	static int GetActiveTicket()
	{
		return s_ActiveTicket;
	}

	/**
	    \brief Whether a ticket is still allowed to act on the breaker.
	    \details Tickets issued before the last state transition are invalid,
	             so stale in-flight callbacks cannot close or trip the breaker.
	    \param ticket Ticket obtained from BeginScrape().
	    \return bool true if the ticket is current.
	*/
	static bool IsTicketValid(int ticket)
	{
		return ticket > s_TicketBarrier;
	}

	/**
	    \brief Report a scrape whose commit (or single POST) was acknowledged.
	    \param ticket Ticket of the reporting scrape.
	*/
	static void ReportScrapeSuccess(int ticket)
	{
		if (!IsTicketValid(ticket))
			return;

		s_ConsecutiveFailures = 0;

		if (s_State == MetricZ_EBreakerState.HALF_OPEN)
			Close();
	}

	/**
	    \brief Report a scrape that failed to reach the backend.
	    \details Counted at most once per ticket: a scrape with several chunks
	             must not count one failure per chunk.
	    \param ticket Ticket of the reporting scrape.
	*/
	static void ReportScrapeFailure(int ticket)
	{
		if (!MetricZ_Config.IsLoaded())
			return;

		int threshold = MetricZ_Config.Get().http.breaker_failures;
		if (threshold <= 0)
			return;

		if (!IsTicketValid(ticket))
			return;

		if (ticket == s_LastFailedTicket)
			return;
		s_LastFailedTicket = ticket;

		if (s_State == MetricZ_EBreakerState.HALF_OPEN) {
			Open();
			return;
		}

		s_ConsecutiveFailures++;
		if (s_ConsecutiveFailures >= threshold)
			Open();
	}

	/**
	    \brief Account for a collection cycle that was skipped while suspended.
	*/
	static void ReportSkippedScrape()
	{
		s_SkippedTotal++;
	}

	/**
	    \brief Result handler for a liveness probe.
	    \details Called by `MetricZ_CallbackHealth`. A 2xx response moves the
	             breaker to HALF_OPEN, anything else schedules the next probe.
	    \param ok true if the backend answered successfully.
	*/
	static void OnProbeResult(bool ok)
	{
		s_Probe = null;

		if (s_State != MetricZ_EBreakerState.OPEN)
			return;

		if (ok) {
			HalfOpen();
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

		s_State = MetricZ_EBreakerState.CLOSED;
		s_TicketBarrier = s_TicketCounter;
		s_ActiveTicket = -1;
		s_LastFailedTicket = -1;
		s_ConsecutiveFailures = 0;
		s_ProbeAttempt = 0;
		s_TrialIssued = false;
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

		s_MetricOpened.Set(s_OpenedTotal);
		s_MetricOpened.FlushWithHead(sink);

		s_MetricSkipped.Set(s_SkippedTotal);
		s_MetricSkipped.FlushWithHead(sink);

		s_MetricSuspendedMs.Set(s_SuspendedMsTotal);
		s_MetricSuspendedMs.FlushWithHead(sink);
	}

	/**
	    \brief Suspend collection and start probing the backend.
	    \details Reached from CLOSED (failure threshold hit) and from
	             HALF_OPEN (trial scrape failed). Reopening keeps the probe
	             attempt counter, so the backoff keeps growing across failed
	             recoveries within the same outage.
	*/
	protected static void Open()
	{
		bool fromClosed = (s_State == MetricZ_EBreakerState.CLOSED);

		s_State = MetricZ_EBreakerState.OPEN;
		s_TicketBarrier = s_TicketCounter;
		s_ActiveTicket = -1;
		s_TrialIssued = false;

		if (fromClosed) {
			s_OpenedTotal++;
			s_ProbeAttempt = 0;

			if (g_Game)
				s_SuspendedSince = g_Game.GetTime();

			ErrorEx(
			    string.Format(
			        "MetricZ: circuit breaker closed -> open after %1 consecutive failed scrapes, probing %2",
			        s_ConsecutiveFailures,
			        MetricZ_Config.Get().http.health_path),
			    ErrorExSeverity.WARNING);
		} else {
			ErrorEx(
			    "MetricZ: circuit breaker half-open -> open, trial scrape failed",
			    ErrorExSeverity.WARNING);
		}

		ScheduleProbe();
	}

	/**
	    \brief A probe succeeded: allow a single trial scrape.
	*/
	protected static void HalfOpen()
	{
		s_State = MetricZ_EBreakerState.HALF_OPEN;
		s_TicketBarrier = s_TicketCounter;
		s_ActiveTicket = -1;
		s_TrialIssued = false;

		if (g_Game)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Probe);

		ErrorEx(
		    "MetricZ: circuit breaker open -> half-open, next scrape verifies the ingest path",
		    ErrorExSeverity.INFO);
	}

	/**
	    \brief The trial scrape was committed: resume normal collection.
	*/
	protected static void Close()
	{
		int downtime = 0;
		if (g_Game) {
			downtime = g_Game.GetTime() - s_SuspendedSince;
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Probe);
		}

		if (downtime > 0)
			s_SuspendedMsTotal += downtime;

		s_State = MetricZ_EBreakerState.CLOSED;
		s_TicketBarrier = s_TicketCounter;
		s_ActiveTicket = -1;
		s_ConsecutiveFailures = 0;
		s_ProbeAttempt = 0;
		s_TrialIssued = false;
		s_Probe = null;

		ErrorEx(
		    string.Format(
		        "MetricZ: circuit breaker half-open -> closed, backend available again after %1 ms",
		        downtime),
		    ErrorExSeverity.INFO);
	}

	/**
	    \brief Deadline for an issued trial scrape to resolve.
	    \details Three collect intervals cover a full scrape including retry
	             chains; never below one minute.
	    \return int Timeout in milliseconds.
	*/
	protected static int TrialTimeout()
	{
		int timeout = MetricZ_Config.Get().settings.collect_interval_sec * 3000;
		if (timeout < 60000)
			timeout = 60000;

		return timeout;
	}

	/**
	    \brief Queue the next liveness probe using exponential backoff with jitter.
	    \details Jitter is applied before the clamp, so the final delay never
	             exceeds `MetricZ_Constants.BREAKER_MAX_DELAY_MS`.
	*/
	protected static void ScheduleProbe()
	{
		if (!g_Game || !MetricZ_Config.IsLoaded())
			return;

		MetricZ_ConfigDTO_HttpExport cfg = MetricZ_Config.Get().http;

		int backoff = cfg.breaker_delay_ms;
		if (s_ProbeAttempt > 0 && s_ProbeAttempt < 31)
			backoff = cfg.breaker_delay_ms << s_ProbeAttempt;

		// Shift overflow yields a negative value: fall back to the cap.
		if (backoff <= 0)
			backoff = MetricZ_Constants.BREAKER_MAX_DELAY_MS;

		// Jitter (+/- 25%) so a fleet of servers does not stampede a
		// recovering backend, then clamp to the hard cap.
		int delay = (int)Math.Floor(backoff * Math.RandomFloat(0.75, 1.25));
		delay = (int)Math.Clamp(delay, cfg.breaker_delay_ms, MetricZ_Constants.BREAKER_MAX_DELAY_MS);

		s_ProbeAttempt++;

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(Probe);
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(Probe, delay, false);
	}

	/**
	    \brief Issue a single liveness probe against the backend.
	*/
	protected static void Probe()
	{
		if (s_State != MetricZ_EBreakerState.OPEN)
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
