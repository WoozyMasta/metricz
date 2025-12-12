/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_CallbackPostMetrics : MetricZ_CallbackBase
{
	protected string m_Body;
	protected string m_TxnId;
	protected int m_Idx;
	protected bool m_IsReady;

	void Setup(string body, string txn, int idx)
	{
		m_Body = body;
		m_TxnId = txn;
		m_Idx = idx;
		m_IsReady = true;
	}

	bool IsReady()
	{
		return m_IsReady;
	}

	override protected void SendAgain()
	{
		if (m_TxnId != string.Empty && !MetricZ_RestTransactionManager.IsActive(m_TxnId)) {
#ifdef DIAG
			ErrorEx("MetricZ: Abort retry for stale txn: " + m_TxnId, ErrorExSeverity.INFO);
#endif
			OnDone();
			return;
		}

		if (m_IsReady && m_Client)
			m_Client.PostMetrics(m_Body, m_TxnId, m_Idx, this);
		else
			OnDone();
	}


	override void OnSuccess(string data, int dataSize)
	{
		if (m_TxnId != string.Empty)
			MetricZ_RestTransactionManager.OnChunkSuccess(m_TxnId, m_Idx);

		super.OnSuccess(data, dataSize);
	}
}
#endif
