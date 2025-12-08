/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_RestCallback : RestCallback
{
	protected ref MetricZ_RestClient m_Client;
	protected string m_Body;
	protected int m_Attempt;
	protected int m_StartedAt;

	void MetricZ_RestCallback(MetricZ_RestClient client, string body)
	{
		m_Client = client;
		m_Body = body;
		m_Attempt = 0;
		m_StartedAt = g_Game.GetTime();
	}

	protected string GetDuration()
	{
		return ", time: " + (g_Game.GetTime() - m_StartedAt).ToString() + "ms";
	}

	protected void Retry()
	{
		if (!MetricZ_Config.Get().enableRemoteExport) {
			OnDone();
			return;
		}

		if (m_Attempt >= MetricZ_Config.Get().remoteMaxRetries) {
			ErrorEx("MetricZ REST: all retries failed" + GetDuration(), ErrorExSeverity.ERROR);
			OnDone();
			return;
		}

		m_Attempt++;

		int baseDelay = MetricZ_Config.Get().remoteRetryDelayMs;
		int backoff = baseDelay << (m_Attempt - 1);
		if (backoff > MetricZ_Config.Get().remoteRetryMaxBackoffMs)
			backoff = MetricZ_Config.Get().remoteRetryMaxBackoffMs;

		int delay = Math.Floor(backoff * Math.RandomFloat(0.75, 1.25));

		string attempt = m_Attempt.ToString() + "/" + MetricZ_Config.Get().remoteMaxRetries.ToString();
		ErrorEx("MetricZ REST: retry " + attempt + " after " + delay.ToString() + "ms", ErrorExSeverity.WARNING);

		if (!g_Game) {
			OnDone();
			return;
		}

		g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SendAgain, delay, false);
	}

	protected void SendAgain()
	{
		if (m_Client)
			m_Client.PostBody(m_Body, this);
		else
			OnDone();
	}

	protected void OnDone()
	{
		if (g_Game)
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(SendAgain);

		m_Client = null;
		m_Body = "";
		delete this;
	}

	override void OnError(int errorCode)
	{
		string msg = "MetricZ REST error: " + EnumTools.EnumToString(ERestResultState, errorCode) + GetDuration();
		ErrorEx(msg, ErrorExSeverity.WARNING);
		Retry();
	}

	override void OnTimeout()
	{
		string msg = "MetricZ REST timeout" + GetDuration();
		ErrorEx(msg, ErrorExSeverity.WARNING);
		Retry();
	}

	override void OnSuccess(string data, int dataSize)
	{
#ifdef DIAG
		ErrorEx("MetricZ REST success, size " + dataSize.ToString() + GetDuration(), ErrorExSeverity.INFO);
#endif

		OnDone();
	}
}
#endif
