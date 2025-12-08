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
			ErrorEx("MetricZ REST: rest-url is empty", ErrorExSeverity.ERROR);
			return;
		}

		m_Rest = GetRestApi();
		if (!m_Rest)
			m_Rest = CreateRestApi();

		if (!m_Rest) {
			ErrorEx("MetricZ REST: cannot create RestApi", ErrorExSeverity.ERROR);
			return;
		}

		m_Rest.SetOption(ERestOption.ERESTOPTION_READOPERATION, 60000);
		m_Rest.SetOption(ERestOption.ERESTOPTION_CONNECTION, 5000);
#ifdef DIAG
		m_Rest.EnableDebug(true);
#endif

		m_Ctx = m_Rest.GetRestContext(baseUrl);
		if (!m_Ctx) {
			ErrorEx("MetricZ REST: GetRestContext failed", ErrorExSeverity.ERROR);
			return;
		}

		m_Ctx.SetHeader("Content-Type: text/plain; charset=utf-8");
	}

	void PostBody(string body, RestCallback cb)
	{
		Init();

		if (!m_Ctx) {
			ErrorEx("MetricZ REST: context is null", ErrorExSeverity.ERROR);
			return;
		}

		m_Ctx.POST(cb, "", body);
#ifdef DIAG
		ErrorEx("MetricZ REST POST", ErrorExSeverity.INFO);
#endif
	}
}
#endif
