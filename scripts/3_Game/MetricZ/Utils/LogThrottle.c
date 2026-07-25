/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Rate-limits repetitive `ErrorEx` log lines per logical key.
    \details Each log entry is tagged with a stable key (e.g. `"rest.timeout"` or
             `"rest.error:EREST_ERROR"`). Within the configured throttle window,
             only the first occurrence per key is emitted; subsequent occurrences
             are counted. When the window elapses and a new occurrence arrives,
             a single line is emitted with a `(previously suppressed N similar
             log entries)` suffix and the counter is reset.

             Motivation: DayZ `ErrorEx` writes a full stack trace per call. A
             misconfigured REST target at the default 15s collect interval can
             produce multiple ErrorEx entries per cycle and inflate `crash_*.log`
             into the GB range. Throttling keeps operational visibility while
             capping log volume.

             Throttling is opt-in via `http.log_throttle_ms`. A value of `0`
             restores the original per-call logging behavior.
*/
class MetricZ_LogThrottle
{
	protected static ref map<string, int> s_LastLoggedAt = new map<string, int>(); //!< key -> last emit timestamp (ms)
	protected static ref map<string, int> s_SuppressedCount = new map<string, int>(); //!< key -> suppressed occurrences since last emit

	/**
	    \brief Emit a log line via `ErrorEx`, rate-limited per key.
	    \param key Stable identifier grouping similar log entries.
	    \param message Log message to emit.
	    \param severity `ErrorEx` severity level.
	    \param throttle_ms Minimum interval between emitted lines for the same key.
	                      `<= 0` disables throttling (message is always emitted).
	*/
	static void Emit(string key, string message, ErrorExSeverity severity, int throttle_ms)
	{
		// Throttling disabled -> passthrough behavior identical to a plain ErrorEx call.
		if (throttle_ms <= 0) {
			ErrorEx(message, severity);
			return;
		}

		int now = 0;
		if (g_Game)
			now = g_Game.GetTime();

		int lastAt;
		bool insideWindow = false;
		if (s_LastLoggedAt.Find(key, lastAt)) {
			// Guard against timer wrap-around / restart: only suppress on a
			// monotonically forward time delta within the throttle window.
			if (now >= lastAt && (now - lastAt) < throttle_ms)
				insideWindow = true;
		}

		if (insideWindow) {
			int suppressed = 0;
			s_SuppressedCount.Find(key, suppressed);
			// Set(), not Insert(): Insert() only adds new keys and would leave the
			// counter pinned at its initial value for every following occurrence.
			s_SuppressedCount.Set(key, suppressed + 1);
			return;
		}

		int totalSuppressed = 0;
		s_SuppressedCount.Find(key, totalSuppressed);

		if (totalSuppressed > 0) {
			ErrorEx(
			    string.Format("%1 (previously suppressed %2 similar log entries)", message, totalSuppressed),
			    severity);
			s_SuppressedCount.Set(key, 0);
		} else
			ErrorEx(message, severity);

		s_LastLoggedAt.Set(key, now);
	}

	/**
	    \brief Force-emit any pending suppressed count for a key.
	    \details Useful when a subsystem knows it will stop generating events
	             for a while and wants the tail count in the log immediately.
	    \param key Stable identifier used with prior `Emit` calls.
	    \param severity `ErrorEx` severity level for the flush line.
	    \param context Short human-readable context appended to the flush line.
	*/
	static void Flush(string key, ErrorExSeverity severity, string context)
	{
		int suppressed = 0;
		if (!s_SuppressedCount.Find(key, suppressed) || suppressed <= 0)
			return;

		ErrorEx(
		    string.Format("MetricZ: throttled log '%1' had %2 suppressed entries (%3)", key, suppressed, context),
		    severity);
		s_SuppressedCount.Set(key, 0);
	}

	/**
	    \brief Force-emit pending suppressed counts for every known key.
	    \details Called when the situation that produced the spam ends (backend
	             recovered, exporter shutting down), so the tail counts are not
	             silently lost.
	    \param severity `ErrorEx` severity level for the flush lines.
	    \param context Short human-readable context appended to each flush line.
	*/
	static void FlushAll(ErrorExSeverity severity, string context)
	{
		// Snapshot the keys: Flush() writes back into the map we would iterate.
		array<string> keys = s_SuppressedCount.GetKeyArray();
		if (!keys)
			return;

		for (int i = 0; i < keys.Count(); ++i)
			Flush(keys.Get(i), severity, context);
	}
}
#endif
