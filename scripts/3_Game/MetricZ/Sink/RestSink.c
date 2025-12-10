/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
class MetricZ_RestSink : MetricZ_SinkBase
{
	private string m_TxnId;
	private ref array<bool> m_Chunks;
	private ref MetricZ_RestClient m_Client;

	void ~MetricZ_RestSink()
	{
#ifdef DIAG
		ErrorEx("MetricZ: RestSink destroyed", ErrorExSeverity.INFO);
#endif
	}

	string GetTransactionID()
	{
		return m_TxnId;
	}

	int GetChunkCount()
	{
		return m_Chunks.Count();
	}

	void SetChunkDone(int id)
	{
		m_Chunks.Set(id, true);
	}

	bool IsAllChunksDone()
	{
		foreach (bool chunk : m_Chunks) {
			if (!chunk)
				return false;
		}

		return true;
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
		} else
			m_TxnId = string.Empty;

		m_Chunks = new array<bool>();

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

		m_TxnId = string.Empty;
		m_Chunks.Clear();

		return super.End();
	}

	override protected void BufferFlush()
	{
		if (m_Client && GetBufferCount() > 0) {
			MetricZ_CallbackPostMetrics cb = new MetricZ_CallbackPostMetrics(m_Client);
			if (!cb)
				ErrorEx("MetricZ: callback not created", ErrorExSeverity.ERROR);
			else {
				m_Chunks.Insert(false);
				m_Client.PostMetrics(this, cb);
			}
		}

		super.BufferFlush();
	}
}
#endif
