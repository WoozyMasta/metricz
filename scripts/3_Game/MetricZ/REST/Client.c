/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief REST Client for communicating with the MetricZ backend.
    \details Handles initialization of the RestApi, setting up headers/timeouts,
             and performing POST requests for ingesting chunks and committing transactions.
*/
class MetricZ_RestClient : Managed
{
	protected static ref MetricZ_RestClient s_Instance; // Singleton instance
	protected RestApi m_Rest; // Native engine RestApi handle
	protected RestContext m_Ctx; // Context for the specific base URL

	/**
	    \brief Singleton accessor.
	    \return Global instance of MetricZ_RestClient or null if config is not loaded.
	*/
	static MetricZ_RestClient Get()
	{
		if (!MetricZ_Config.IsLoaded())
			return null;

		if (!s_Instance)
			s_Instance = new MetricZ_RestClient();

		return s_Instance;
	}

	/**
	    \brief Lazy initialization of the REST context.
	    \details Configures the RestApi with timeouts and headers from the configuration.
	             This method ensures we don't recreate the context on every request.
	*/
	protected void Init()
	{
		if (m_Ctx)
			return;

		string url = MetricZ_Config.Get().http.url;

		if (!MetricZ_Config.Get().http.enabled) {
			ErrorEx("MetricZ: rest is disabled", ErrorExSeverity.WARNING);
			return;
		}

		// Get or Create the native RestApi manager
		m_Rest = GetRestApi();
		if (!m_Rest)
			m_Rest = CreateRestApi();

		if (!m_Rest) {
			ErrorEx("MetricZ: cannot create RestApi", ErrorExSeverity.ERROR);
			return;
		}

		m_Rest.SetOption(ERestOption.ERESTOPTION_READOPERATION, MetricZ_Config.Get().http.read_timeout_sec);
		m_Rest.SetOption(ERestOption.ERESTOPTION_CONNECTION, MetricZ_Config.Get().http.connect_timeout_sec);
#ifdef DIAG
		m_Rest.EnableDebug(true);
#endif

		// Initialize the context for the target URL
		m_Ctx = m_Rest.GetRestContext(url);
		if (!m_Ctx) {
			ErrorEx("MetricZ: failed in GetRestContext", ErrorExSeverity.ERROR);
			return;
		}

		m_Ctx.SetHeader("text/plain");
	}

	/**
	    \brief Send a single chunk of metrics to the backend.
	    \param body The payload (Prometheus formatted metrics)
	    \param txn The transaction ID to associate this chunk with
	    \param chunk The sequence number of this chunk (0, 1, 2...)
	    \param cb The callback handler for success/retry logic
	*/
	void PostMetrics(string body, string txn, int chunk, MetricZ_CallbackPostMetrics cb)
	{
		if (!cb || body == string.Empty)
			return;

		Init();

		// Construct URL: /api/v1/ingest/<instance_id>/<txn_id>/<seq_id>
		// The backend uses <seq_id> to reassemble chunks in the correct order.
		string url = "/api/v1/ingest/" + MetricZ_Config.Get().settings.instance_id;
		if (txn != string.Empty)
			url += "/" + txn + "/" + chunk.ToString();

		// If callback is fresh (not a retry), configure it with current data
		if (!cb.IsReady())
			cb.Setup(body, txn, chunk);

		MetricZ_HttpStats.AddBytes(body.Length());
		m_Ctx.POST(cb, url, body);

#ifdef DIAG
		ErrorEx("MetricZ: metrics POST " + url, ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Signal the backend that all chunks for a transaction have been sent.
	    \param txn The transaction ID to commit
	    \param cb Callback handler
	*/
	void CommitMetrics(string txn, MetricZ_CallbackCommitMetrics cb)
	{
		if (txn == string.Empty || !cb)
			return;

		Init();

		string url = "/api/v1/commit/" + MetricZ_Config.Get().settings.instance_id + "/" + txn;

		cb.SetTxn(txn);
		m_Ctx.POST(cb, url, string.Empty);

#ifdef DIAG
		ErrorEx("MetricZ: commit POST " + url, ErrorExSeverity.INFO);
#endif
	}
}
#endif
