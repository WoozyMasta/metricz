/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Callback handler for uploading metric chunks.
    \details Handles the lifecycle of a single chunk upload.
             Includes logic to check against `TransactionManager` to prevent "zombie" retries
             if the transaction has already been superseded.
*/
class MetricZ_CallbackPostMetrics : MetricZ_CallbackBase
{
	protected string m_Body; //!< Prometheus text payload
	protected string m_TxnId; //!< Associated transaction ID
	protected int m_Idx; //!< Sequence number of the chunk
	protected bool m_IsReady; //!< Flag indicating if data is populated

	/**
	    \brief Configures the callback with payload and metadata.
	*/
	void Setup(string body, string txn, int idx)
	{
		m_Body = body;
		m_TxnId = txn;
		m_Idx = idx;
		m_IsReady = true;
	}

	/**
	    \brief Checks if the callback has been configured.
	*/
	bool IsReady()
	{
		return m_IsReady;
	}

	/**
	    \brief Retries the chunk upload.
	*/
	override protected void SendAgain()
	{
		// Race condition protection
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

	/**
	    \brief Reports successful upload to the Transaction Manager.
	*/
	override void OnSuccess(string data, int dataSize)
	{
		if (m_TxnId != string.Empty)
			MetricZ_RestTransactionManager.OnChunkSuccess(m_TxnId, m_Idx);

		super.OnSuccess(data, dataSize);
	}
}
#endif
