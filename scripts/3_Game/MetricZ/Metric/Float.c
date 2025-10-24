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
	    \param fh Open file handle
	    \param labels Optional labels
	*/
	override void Flush(FileHandle fh, string labels = "")
	{
		if (!fh)
			return;

		FPrint(fh, m_Name + labels + " " + m_Value.ToString() + "\n");
	}

	/**
	    \brief Write HELP/TYPE then value.
	    \param fh Open file handle
	    \param labels Optional labels
	*/
	override void FlushWithHead(FileHandle fh, string labels = "")
	{
		if (!fh)
			return;

		WriteHeaders(fh);
		Flush(fh, labels);
	}
}
#endif
