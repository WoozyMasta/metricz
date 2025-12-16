/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Base class for handling REST API callbacks.
    \details Implements common logic for request timing, error handling, statistics collection,
             and exponential backoff for retrying failed requests.
             Self-managed lifecycle: deletes itself upon completion (`OnDone`).
*/
class MetricZ_CallbackBase : RestCallback
{
	private int m_Attempt; //!< Current retry attempt counter
	private int m_StartedAt; //!< Timestamp when the callback was created (request started)
	protected ref MetricZ_RestClient m_Client; //!< Reference to client for re-sending requests
	protected string m_ReqType; //!< Class name of the callback (used for stats tagging)

	/**
	    \brief Constructor.
	    \param client Reference to the MetricZ_RestClient instance.
	*/
	void MetricZ_CallbackBase(MetricZ_RestClient client)
	{
		m_Client = client;
		m_Attempt = 0;
		m_StartedAt = g_Game.GetTime();
		m_ReqType = ClassName();
		m_ReqType.Replace("MetricZ_Callback", "");
	}

	void ~MetricZ_CallbackBase()
	{
#ifdef DIAG
		ErrorEx("MetricZ: callback " + ClassName() + " destroyed" + GetDuration(), ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Calculates the duration since the request started.
	*/
	protected string GetDuration()
	{
		return ", time: " + (g_Game.GetTime() - m_StartedAt).ToString() + "ms";
	}

	/**
	    \brief Handles retry logic with exponential backoff and jitter.
	    \details Checks config limits. If retries remain, schedules `SendAgain` via CallQueue.
	             Delay = BaseDelay * 2^(Attempt) * Random(0.75, 1.25).
	*/
	protected void Retry()
	{
		if (m_Attempt >= MetricZ_Config.Get().http.max_retries) {
			ErrorEx("MetricZ: callback REST all retries failed" + GetDuration(), ErrorExSeverity.ERROR);
			OnDone();
			return;
		}

		m_Attempt++;
		MetricZ_HttpStats.IncRetry();

		// calculate exponential backoff
		int baseDelay = MetricZ_Config.Get().http.retry_delay_ms;
		int backoff = baseDelay << (m_Attempt - 1);
		if (backoff > MetricZ_Config.Get().http.retry_max_backoff_ms)
			backoff = MetricZ_Config.Get().http.retry_max_backoff_ms;

		// add random jitter (+/- 25%) to prevent thundering herd
		int delay = Math.Floor(backoff * Math.RandomFloat(0.75, 1.25));

		string attempt = m_Attempt.ToString() + "/" + MetricZ_Config.Get().http.max_retries.ToString();
		ErrorEx("MetricZ: callback REST retry " + attempt + " after " + delay.ToString() + "ms", ErrorExSeverity.WARNING);

		if (!g_Game) {
			OnDone();
			return;
		}

		// schedule the retry
		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SendAgain, delay, false);
	}

	/**
	    \brief Virtual method to re-trigger the specific request.
	*/
	protected void SendAgain()
	{
		OnDone();
	}

	/**
	    \brief Finalizes the callback lifecycle.
	    \details Removes any pending calls and destroys the object.
	             WARNING: 'delete this' is called here. Do not access members after calling OnDone.
	*/
	protected void OnDone()
	{
		if (g_Game)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(SendAgain);

		m_Client = null;
		delete this;
	}

	/**
	    \brief Handler for REST API errors (transport/protocol errors).
	*/
	override void OnError(int errorCode)
	{
		MetricZ_HttpStats.IncRequest(m_ReqType, "error");
		string msg = "MetricZ: callback REST error: " + EnumTools.EnumToString(ERestResultState, errorCode) + GetDuration();
		ErrorEx(msg, ErrorExSeverity.WARNING);
		Retry();
	}

	/**
	    \brief Handler for REST API timeouts.
	*/
	override void OnTimeout()
	{
		MetricZ_HttpStats.IncRequest(m_ReqType, "timeout");
		string msg = "MetricZ: callback REST timeout" + GetDuration();
		ErrorEx(msg, ErrorExSeverity.WARNING);
		Retry();
	}

	/**
	    \brief Handler for successful REST API responses (HTTP 200-299).
	*/
	override void OnSuccess(string data, int dataSize)
	{
#ifdef DIAG
		ErrorEx("MetricZ: callback REST success, size " + dataSize.ToString() + GetDuration(), ErrorExSeverity.INFO);
#endif

		MetricZ_HttpStats.IncRequest(m_ReqType, "success");
		OnDone();
	}
}
#endif
