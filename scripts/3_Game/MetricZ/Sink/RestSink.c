/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Sink implementation for HTTP REST export.
    \details Buffers metrics and sends them in chunks to `MetricZ_RestClient`.
             Delegates transaction state management to `MetricZ_RestTransactionManager`.
*/
class MetricZ_RestSink : MetricZ_SinkBase
{
	private string m_TxnId;
	private ref MetricZ_RestClient m_Client;

	/**
	    \brief Begins a new transaction.
	    \details Generates a UUID and initializes the static `TransactionManager`.
	*/
	override bool Begin()
	{
		if (!MetricZ_Config.IsLoaded())
			return false;

		if (!IsBuffered())
			return false;

		if (!super.Begin())
			return false;

		m_Client = MetricZ_RestClient.Get();
		if (!m_Client)
			return false;

		if (GetBufferLimit() > 0) {
			int uuid[4];
			UUIDApi.Generate(uuid);
			m_TxnId = UUIDApi.FormatString(uuid);
			MetricZ_RestTransactionManager.Start(m_TxnId);
		}

		return true;
	}

	override void Line(string line)
	{
		if (m_Client)
			super.Line(line);
	}

	/**
	    \brief Ends the transaction.
	    \details Flushes remaining buffer and tells `TransactionManager` to Seal the transaction.
	*/
	override bool End()
	{
		if (!m_Client)
			return false;

		BufferFlush();

		if (m_TxnId != string.Empty)
			MetricZ_RestTransactionManager.Seal(m_TxnId);

		m_TxnId = string.Empty;

		return super.End();
	}

	/**
	    \brief Flushes buffer as a single HTTP request chunk.
	    \details Registers the chunk with `TransactionManager` to get a sequence ID.
	*/
	override protected void BufferFlush()
	{
		if (m_Client && GetBufferCount() > 0) {
			int chunkIdx = -1;
			if (m_TxnId != string.Empty)
				chunkIdx = MetricZ_RestTransactionManager.AddChunk(m_TxnId);

			MetricZ_CallbackPostMetrics cb = new MetricZ_CallbackPostMetrics(m_Client);
			if (!cb)
				ErrorEx("MetricZ: callback not created", ErrorExSeverity.ERROR);
			else
				m_Client.PostMetrics(GetBufferChunk(), m_TxnId, chunkIdx, cb);
		}

		super.BufferFlush();
	}
}
#endif
