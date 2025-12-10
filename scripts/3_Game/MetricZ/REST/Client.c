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

		string baseUrl = MetricZ_Config.Get().remoteEndpointURL;
		if (baseUrl == string.Empty) {
			ErrorEx("MetricZ: rest-url is empty", ErrorExSeverity.ERROR);
			return;
		}

		m_Rest = GetRestApi();
		if (!m_Rest)
			m_Rest = CreateRestApi();

		if (!m_Rest) {
			ErrorEx("MetricZ: cannot create RestApi", ErrorExSeverity.ERROR);
			return;
		}

		m_Rest.SetOption(ERestOption.ERESTOPTION_READOPERATION, MetricZ_Config.Get().remoteReadTimeout);
		m_Rest.SetOption(ERestOption.ERESTOPTION_CONNECTION, MetricZ_Config.Get().remoteConnectionTimeout);
#ifdef DIAG
		m_Rest.EnableDebug(true);
#endif

		m_Ctx = m_Rest.GetRestContext(baseUrl);
		if (!m_Ctx) {
			ErrorEx("MetricZ: GetRestContext failed", ErrorExSeverity.ERROR);
			return;
		}

		m_Ctx.SetHeader("text/plain");
	}

	void PostMetrics(MetricZ_RestSink sink, MetricZ_CallbackPostMetrics cb)
	{
		if (!sink || !cb)
			return;

		Init();

		string url = "/api/v1/ingest/" + MetricZ_InstanceID.Get();

		string txn = sink.GetTransactionID();
		if (txn != string.Empty)
			url += "/" + txn;

		string body;
		if (!cb.IsConfigured()) {
			body = sink.GetBufferChunk();
			cb.SetBody(body);
			cb.SetSink(sink);
		} else
			body = cb.GetBody();

		m_Ctx.POST(cb, url, body);

#ifdef DIAG
		ErrorEx("MetricZ: POST metrics " + url, ErrorExSeverity.INFO);
#endif
	}

	void CommitMetrics(string txn, MetricZ_CallbackCommitMetrics cb)
	{
		if (txn == string.Empty || !cb)
			return;

		Init();

		string url = "/api/v1/commit/" + MetricZ_InstanceID.Get() + "/" + txn;

		cb.SetTxn(txn);
		m_Ctx.POST(cb, url, string.Empty);

#ifdef DIAG
		ErrorEx("MetricZ: POST commit " + url, ErrorExSeverity.INFO);
#endif
	}
}
#endif
