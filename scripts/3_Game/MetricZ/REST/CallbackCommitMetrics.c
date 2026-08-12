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
	    \param txn Transaction ID to commit
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

	/**
	    \brief The backend acknowledged the commit: the scrape succeeded.
	    \param data Response data
	    \param dataSize Response data size
	*/
	override void OnSuccess(string data, int dataSize)
	{
		MetricZ_RestCircuitBreaker.ReportScrapeSuccess(m_Ticket);
		super.OnSuccess(data, dataSize);
	}

	/**
	    \brief The commit exhausted its retries: the scrape has failed.
	*/
	override protected void OnFinalFailure()
	{
		MetricZ_RestCircuitBreaker.ReportScrapeFailure(m_Ticket);
	}
}
#endif
