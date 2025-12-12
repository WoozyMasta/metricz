/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Callback handler for the final 'Commit' request.
    \details Triggers the backend to merge all uploaded chunks for a specific transaction ID.
*/
class MetricZ_CallbackCommitMetrics: MetricZ_CallbackBase
{
	protected string m_Txn; //!< Transaction ID to commit

	/**
	    \brief Sets the transaction ID for this commit request.
	*/
	void SetTxn(string txn)
	{
		m_Txn = txn;
	}

	/**
	    \brief Retries the commit request.
	*/
	override protected void SendAgain()
	{
		if (m_Client && m_Txn != string.Empty)
			m_Client.CommitMetrics(m_Txn, this);
		else
			OnDone();
	}
}
#endif
