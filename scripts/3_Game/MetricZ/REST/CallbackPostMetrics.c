/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_CallbackPostMetrics : MetricZ_CallbackBase
{
	protected int m_Idx;
	protected string m_Body;
	protected bool m_IsConfigured;
	protected ref MetricZ_RestSink m_Sink;

	void SetBody(string body)
	{
		if (body == string.Empty)
			return;

		m_Body = body;
		m_IsConfigured = true;
	}

	void SetSink(MetricZ_RestSink sink)
	{
		if (!sink)
			return;

		m_Sink = sink;
		m_Idx = sink.GetChunkCount();
	}

	bool IsConfigured()
	{
		return m_IsConfigured;
	}

	string GetBody()
	{
		return m_Body;
	}

	override protected void SendAgain()
	{
		if (m_IsConfigured && m_Client)
			m_Client.PostMetrics(m_Sink, this);
		else
			OnDone();
	}

	override protected void OnDone()
	{
		m_Sink = null;

		super.OnDone();
	}

	override void OnSuccess(string data, int dataSize)
	{
		if (!m_Sink) {
			super.OnSuccess(data, dataSize);
			return;
		}

		string txn = m_Sink.GetTransactionID();
		if (txn == string.Empty) {
			super.OnSuccess(data, dataSize);
			return;
		}

		m_Sink.SetChunkDone(m_Idx);
		if (!m_Sink.IsAllChunksDone()) {
			super.OnSuccess(data, dataSize);
			return;
		}

		if (!m_Client) {
			ErrorEx("MetricZ: client not found", ErrorExSeverity.ERROR);
			OnDone();
			return;
		}

		MetricZ_CallbackCommitMetrics cb = new MetricZ_CallbackCommitMetrics(m_Client);
		m_Client.CommitMetrics(txn, cb);

		super.OnSuccess(data, dataSize);
	}
}
#endif
