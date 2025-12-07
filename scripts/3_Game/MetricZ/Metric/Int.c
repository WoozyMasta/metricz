/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Integer metric for gauges and counters.
*/
class MetricZ_MetricInt : MetricZ_MetricBase
{
	protected int m_Value; //!< Current value

	/**
	    \brief Get current value.
	    \return \p int
	*/
	int Get()
	{
		return m_Value;
	}

	/**
	    \brief Set value.
	    \param x New value
	*/
	void Set(int x)
	{
		m_Value = x;
	}

	/**
	    \brief Increment by 1.
	*/
	void Inc()
	{
		m_Value++;
	}

	/**
	    \brief Add signed delta.
	    \param d Delta
	*/
	void Add(int d)
	{
		m_Value = m_Value + d;
	}

	/**
	    \brief Decrement by 1.
	*/
	void Dec()
	{
		m_Value--;
	}

	/**
	    \brief Write value line.
	    \param fh Open file handle
	    \param labels Optional labels override, if blank try use internal labels
	*/
	override void Flush(FileHandle fh, string labels = "")
	{
		if (!fh)
			return;

		if (labels == string.Empty)
			labels = GetLabels();

		FPrint(fh, m_Name + labels + " " + m_Value.ToString() + "\n");
		// TODO sink.Line(m_Name + labels + " 0");
	}

	/**
	    \brief Write HELP/TYPE then value.
	    \param fh Open file handle
	    \param labels Optional labels override, if blank try use internal labels
	*/
	override void FlushWithHead(FileHandle fh, string labels = "")
	{
		if (!fh)
			return;

		if (labels == string.Empty)
			labels = GetLabels();

		WriteHeaders(fh);
		Flush(fh, labels);
	}
}
#endif
