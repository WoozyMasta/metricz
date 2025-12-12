/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Floating-point gauge metric.
    \details For continuous values like FPS or temperature.
*/
class MetricZ_MetricFloat : MetricZ_MetricBase
{
	protected float m_Value; //!< Current value

	/**
	    \brief Get current value.
	    \return \p float
	*/
	float Get()
	{
		return m_Value;
	}

	/**
	    \brief Set value.
	    \param x New value
	*/
	void Set(float x)
	{
		m_Value = x;
	}

	/**
	    \brief Add value.
	    \param x New value
	*/
	void Add(float x)
	{
		m_Value += x;
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
