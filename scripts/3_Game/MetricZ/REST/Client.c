/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_RestClient : Managed
{
	protected static ref MetricZ_RestClient s_Instance;
	protected RestApi m_Rest;
	protected RestContext m_Ctx;

	static MetricZ_RestClient Get()
	{
		if (!MetricZ_Config.IsLoaded())
			return null;

		if (!s_Instance)
			s_Instance = new MetricZ_RestClient();

		return s_Instance;
	}

	protected void Init()
	{
		if (m_Ctx)
			return;

		string url = MetricZ_Config.Get().http.url;

		if (!MetricZ_Config.Get().http.enabled) {
			ErrorEx("MetricZ: rest is disabled", ErrorExSeverity.WARNING);
			return;
		}

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

		m_Ctx = m_Rest.GetRestContext(url);
		if (!m_Ctx) {
			ErrorEx("MetricZ: failed in GetRestContext", ErrorExSeverity.ERROR);
			return;
		}

		m_Ctx.SetHeader("text/plain");
	}

	void PostMetrics(string body, string txn, int chunk, MetricZ_CallbackPostMetrics cb)
	{
		if (!cb || body == string.Empty)
			return;

		Init();

		string url = "/api/v1/ingest/" + MetricZ_Config.Get().settings.instance_id;
		if (txn != string.Empty)
			url += "/" + txn + "/" + chunk.ToString();

		if (!cb.IsReady())
			cb.Setup(body, txn, chunk);

		MetricZ_HttpStats.AddBytes(body.Length());
		m_Ctx.POST(cb, url, body);

#ifdef DIAG
		ErrorEx("MetricZ: metrics POST " + url, ErrorExSeverity.INFO);
#endif
	}

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
