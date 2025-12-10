/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_CallbackCommitMetrics: MetricZ_CallbackBase
{
	protected string m_Txn;

	void SetTxn(string txn)
	{
		m_Txn = txn;
	}

	override protected void SendAgain()
	{
		if (m_Client && m_Txn != string.Empty)
			m_Client.CommitMetrics(m_Txn, this);
		else
			OnDone();
	}
}
#endif
