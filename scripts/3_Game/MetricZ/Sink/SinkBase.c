/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Abstract base class for metric sinks.
*/
class MetricZ_SinkBase
{
	private ref array<string> m_Buffer;
	private int m_BufferLimit;
	private bool m_IsBuffered;
	private bool m_Busy;

	// 0 - flush every line, no buffer
	// positive - flush on limit reach
	// negative - unlimited, need flush manual
	void MetricZ_SinkBase(int bufferLimit)
	{
		if (bufferLimit != 0) {
			m_BufferLimit = bufferLimit;
			m_IsBuffered = true;
			m_Buffer = new array<string>();
		}
	}

	/**
	    \brief Prepare sink for writing a new batch.
	    \return bool True if ready, false if busy or error.
	*/
	bool Begin()
	{
		if (IsBusy())
			return false;

		BufferFlush();
		m_Busy = true;

#ifdef DIAG
		ErrorEx("MetricZ: Sink: begin", ErrorExSeverity.INFO);
#endif

		return true;
	}

	/**
	    \brief Write a single metric line.
	*/
	void Line(string line)
	{
		if (IsBusy() && IsBuffered())
			BufferInsert(line);
	}

	/**
	    \brief Finalize batch (flush/send).
	    \return bool True on success.
	*/
	bool End()
	{
		if (!IsBusy())
			return false;

		BufferFlush();
		m_Busy = false;

#ifdef DIAG
		ErrorEx("MetricZ: Sink: end", ErrorExSeverity.INFO);
#endif

		return true;
	}

	bool IsBusy()
	{
		return m_Busy;
	}

	bool IsBuffered()
	{
		return (m_IsBuffered && m_Buffer);
	}

	int GetBufferLimit()
	{
		return m_BufferLimit;
	}

	int GetBufferCount()
	{
		if (!IsBuffered())
			return -1;

		return m_Buffer.Count();
	}

	string GetBufferChunk()
	{
		if (!IsBuffered() || m_Buffer.Count() == 0)
			return string.Empty;

		string chunk;
		foreach (string line : m_Buffer)
			chunk += line + "\n";

		return chunk;
	}

	protected void BufferInsert(string line)
	{
		if (!IsBusy() || !IsBuffered())
			return;

		m_Buffer.Insert(line);

		if (m_Buffer.Count() >= m_BufferLimit)
			BufferFlush();
	}

	protected void BufferFlush()
	{
		if (IsBuffered())
			m_Buffer.Clear();
	}
}
#endif
