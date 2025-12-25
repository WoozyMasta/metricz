/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Abstract base class for metric sinks.
    \details Implements common state management (Busy/Idle) and string buffering logic.
             Derived classes should override methods for specific output handling (e.g., File I/O, REST API).
*/
class MetricZ_SinkBase
{
	private int m_BufferLimit;
	private bool m_IsBuffered;
	private bool m_Busy;
	private ref array<string> m_Buffer;

	private static ref JsonSerializer s_Serializer;

	void ~MetricZ_SinkBase()
	{
#ifdef DIAG
		ErrorEx("MetricZ: destroyed sink " + ClassName(), ErrorExSeverity.INFO);
#endif
	}

	/**
	    \brief Constructor for the base sink.
	    \param bufferLimit Defines the buffering strategy:
	           0 - No buffering (flush every line immediately).
	           > 0 - Buffered (flush automatically when the limit is reached).
	           < 0 - Unlimited buffer (flush only on End() or manual call).
	*/
	void SetBuffer(int bufferLimit)
	{
		if (bufferLimit != 0) {
			m_BufferLimit = bufferLimit;
			m_IsBuffered = true;
			m_Buffer = new array<string>();
			if (bufferLimit > 0)
				m_Buffer.Reserve(bufferLimit);
			else
				m_Buffer.Reserve(MetricZ_Constants.SINK_BUFFER_PREALLOC);
		}
	}

	/**
	    \brief Prepare the sink for writing a new batch of metrics (transaction start).
	    \details Clears previous buffers and sets the busy state.
	    \return bool True if the sink is ready, false if it is already busy or failed to initialize.
	*/
	bool Begin()
	{
		if (IsBusy())
			return false;

		BufferFlush();
		m_Busy = true;

#ifdef DIAG
		ErrorEx("MetricZ: sink: begin", ErrorExSeverity.INFO);
#endif

		return true;
	}

	/**
	    \brief Write a single metric line.
	    \details If buffering is enabled, the line is added to the internal array.
	             If buffering is disabled, this base method does nothing (must be overridden).
	    \param line The metric string (usually in Prometheus format).
	*/
	void Line(string line)
	{
		if (IsBusy() && IsBuffered())
			BufferInsert(line);
	}

	/**
	    \brief Finalize the batch writing (transaction end).
	    \details Flushes any remaining data in the buffer and resets the busy state.
	    \return bool True on success.
	*/
	bool End()
	{
		if (!IsBusy())
			return false;

		BufferFlush();
		m_Busy = false;

#ifdef DIAG
		ErrorEx("MetricZ: sink: end", ErrorExSeverity.INFO);
#endif

		return true;
	}

	/**
	    \brief Check if the sink is currently processing a batch (between Begin and End).
	*/
	bool IsBusy()
	{
		return m_Busy;
	}

	/**
	    \brief Check if buffering is enabled for this instance.
	*/
	bool IsBuffered()
	{
		return (m_IsBuffered && m_Buffer);
	}

	/**
	    \brief Get the configured buffer limit.
	*/
	int GetBufferLimit()
	{
		return m_BufferLimit;
	}

	/**
	    \brief Get the current number of lines in the buffer.
	    \return int Count of lines, or -1 if buffering is disabled.
	*/
	int GetBufferCount()
	{
		if (!IsBuffered())
			return -1;

		return m_Buffer.Count();
	}

	/**
	    \brief Combine all buffered lines into a single string chunk.
	    \details Joins the array elements with newline characters (\n).
	    \return string The complete buffered text.
	*/
	string GetBufferChunk()
	{
		if (!IsBuffered() || m_Buffer.Count() == 0)
			return string.Empty;

		string chunk;
		foreach (string line : m_Buffer)
			chunk += string.Format("%1\n", line);

		return chunk;
	}

	/**
	    \brief Combine all buffered lines into a single JSON array.
	    \return string The complete one line json text string.
	*/
	string GetJsonBufferChunk()
	{
		if (!IsBuffered() || m_Buffer.Count() == 0)
			return string.Empty;

		string json;
		if (!s_Serializer)
			s_Serializer = new JsonSerializer();

		if (!s_Serializer.WriteToString(m_Buffer, false, json)) {
			ErrorEx("MetricZ: sink json serialization failed", ErrorExSeverity.ERROR);
			return string.Empty;
		}

		return json;
	}

	/**
	    \brief Internal helper to add a line to the buffer.
	    \details Automatically calls BufferFlush() if the buffer limit is reached.
	*/
	protected void BufferInsert(string line)
	{
		if (!IsBusy() || !IsBuffered())
			return;

		m_Buffer.Insert(line);

		if (m_Buffer.Count() >= m_BufferLimit && m_BufferLimit > 0)
			BufferFlush();
	}

	/**
	    \brief Clear the buffer memory.
	    \details Derived classes must override this to write/send data before clearing.
	             Always call super.BufferFlush() at the end of the override.
	*/
	protected void BufferFlush()
	{
		if (IsBuffered())
			m_Buffer.Clear();
	}
}
#endif
