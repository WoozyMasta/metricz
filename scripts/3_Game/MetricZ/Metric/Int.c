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
	    \param MetricZ_SinkBase sink instance
	    \param labels Optional labels override, if blank try use internal labels
	*/
	override void Flush(MetricZ_SinkBase sink, string labels = "")
	{
		if (!sink)
			return;

		if (labels == string.Empty)
			labels = GetLabels();

		sink.Line(m_Name + labels + " " + m_Value.ToString());
	}

	/**
	    \brief Write HELP/TYPE then value.
	    \param MetricZ_SinkBase sink instance
	    \param labels Optional labels override, if blank try use internal labels
	*/
	override void FlushWithHead(MetricZ_SinkBase sink, string labels = "")
	{
		if (!sink)
			return;

		if (labels == string.Empty)
			labels = GetLabels();

		WriteHeaders(sink);
		Flush(sink, labels);
	}
}
#endif
