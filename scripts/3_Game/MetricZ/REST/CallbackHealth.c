/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Callback handler for backend liveness probes.
    \details Intentionally does NOT derive from `MetricZ_CallbackBase`: a probe must
             never retry, never emit per-failure log lines and never count towards
             transport statistics. Retry pacing is owned by
             `MetricZ_RestCircuitBreaker`, which schedules the next probe itself.

             Lifetime is owned by the circuit breaker (it holds the only strong
             reference), so this class must not `delete this`.
*/
class MetricZ_CallbackHealth : RestCallback
{
	/**
	    \brief Backend answered with 2xx - the exporter is reachable again.
	    \param data Response data
	    \param dataSize Response data size
	*/
	override void OnSuccess(string data, int dataSize)
	{
		MetricZ_RestCircuitBreaker.OnProbeResult(true);
	}

	/**
	    \brief Transport or protocol error - backend still unavailable.
	    \param errorCode Error code
	*/
	override void OnError(int errorCode)
	{
		MetricZ_RestCircuitBreaker.OnProbeResult(false);
	}

	/**
	    \brief Probe timed out - backend still unavailable.
	*/
	override void OnTimeout()
	{
		MetricZ_RestCircuitBreaker.OnProbeResult(false);
	}
}
#endif
