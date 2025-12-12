/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_RestSink : MetricZ_SinkBase
{
	private string m_TxnId;
	private ref MetricZ_RestClient m_Client;

	string GetTransactionID()
	{
		return m_TxnId;
	}

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
