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
	private int m_Ticket = -1; //!< Circuit breaker ticket drawn by the exporter for this scrape
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

		// Ticket drawn by the exporter for this scrape. Invalid while the
		// breaker refuses REST traffic (open, or a trial already running):
		// keep collecting for the file sink, but never open a transaction.
		m_Ticket = MetricZ_RestCircuitBreaker.GetActiveTicket();

		if (MetricZ_RestCircuitBreaker.IsTicketValid(m_Ticket) && GetBufferLimit() > 0) {
			int uuid[4];
			UUIDApi.Generate(uuid);
			m_TxnId = UUIDApi.FormatString(uuid);
			MetricZ_RestTransactionManager.Start(m_TxnId, m_Ticket);
		}

		return true;
	}

	/**
	    \brief Writes a metric line to the REST sink.
	    \param line Metric line to write
	*/
	override void Line(string line)
	{
		if (m_Client)
			super.Line(line);
	}

	/**
	    \brief Ends the transaction.
	    \details Flushes the remaining buffer and seals the transaction.
	             If the ticket became invalid, BufferFlush() has already
	             aborted the transaction and cleared m_TxnId, so a partial
	             scrape is never sealed here.
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
	    \details Registers the chunk with `TransactionManager` to get a
	             sequence ID. If the breaker invalidated the ticket before or
	             during this scrape, the active transaction is aborted so a
	             partially uploaded scrape can never be sealed or committed.
	*/
	override protected void BufferFlush()
	{
		if (!MetricZ_RestCircuitBreaker.IsTicketValid(m_Ticket)) {
			if (m_TxnId != string.Empty) {
				MetricZ_RestTransactionManager.Abort(m_TxnId);
				m_TxnId = string.Empty;
			}

			super.BufferFlush();
			return;
		}

		if (m_Client && GetBufferCount() > 0) {
			int chunkIdx = -1;
			if (m_TxnId != string.Empty)
				chunkIdx = MetricZ_RestTransactionManager.AddChunk(m_TxnId);

			MetricZ_CallbackPostMetrics cb = new MetricZ_CallbackPostMetrics(m_Client);
			if (!cb)
				ErrorEx("MetricZ: callback not created", ErrorExSeverity.ERROR);
			else {
				cb.SetTicket(m_Ticket);
				if (MetricZ_Config.Get().http.serialized)
					m_Client.PostMetrics(GetJsonBufferChunk(), m_TxnId, chunkIdx, cb);
				else
					m_Client.PostMetrics(GetBufferChunk(), m_TxnId, chunkIdx, cb);
			}

		}

		super.BufferFlush();
	}
}
#endif
