/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/** Metric type */
enum MetricZ_MetricType {
	GAUGE = 0,
	COUNTER = 1
}

/**
    \brief Base class for Prometheus metrics.
    \details Stores name, HELP text and type. Prints HELP/TYPE headers.
*/
class MetricZ_MetricBase
{
	protected string m_Name;
	protected string m_Help;
	protected string m_Labels;
	protected MetricZ_MetricType m_Type;

	/**
	    \brief Constructor.
	    \param name Metric name
	    \param help HELP text
	    \param type Metric type (GAUGE/COUNTER)
	*/
	void MetricZ_MetricBase(string name, string help, MetricZ_MetricType type = 0)
	{
		m_Name = MetricZ_Config.NS + name;
		if (type == MetricZ_MetricType.COUNTER)
			m_Name += "_total";

		m_Help = help;
		m_Type = type;
	}

	/**
	    \brief Set preformatted label block for this metric.
	    \details Accepts a raw Prometheus label string including braces, e.g. "{k=\"v\"}".
	             The string is used as-is: no escaping, merging, or validation is performed.
	             Prefer MakeLabels(...) when you have a map of labels.
	    \param raw Raw label block with surrounding braces, or empty string to clear.
	*/
	void SetLabels(string raw)
	{
		m_Labels = raw;
	}

	/**
	    \brief Build and set labels from a key/value map.
	    \details Uses MetricZ_LabelUtils::MakeLabels(labels) which:
	             - always includes base labels {world,host,instance_id}
	             - escapes values
	             - ignores keys that overlap base labels
	             Passing null or an empty map results in base labels only.
	    \param labels Map of extra labels; may be null.
	*/
	void MakeLabels(map<string, string> labels)
	{
		m_Labels = MetricZ_LabelUtils.MakeLabels(labels);
	}

	/**
	    \brief Get metric name.
	    \return \p string
	*/
	string GetName()
	{
		return m_Name;
	}

	/**
	    \brief Build HELP header.
	    \return \p string "# HELP ..."
	*/
	string GetHelp()
	{
		return "# HELP " + m_Name + " " + m_Help;
	}

	/**
	    \brief Build TYPE header.
	    \return \p string "# TYPE ..."
	*/
	string GetType()
	{
		return "# TYPE " + m_Name + " " + TypeToText();
	}

	/**
	    \brief Get metric type enum.
	    \return \p MetricZ_MetricType
	*/
	MetricZ_MetricType GetMetricType()
	{
		return m_Type;
	}

	/**
	    \brief Get the effective label block for this metric.
	    \details If labels were set via SetLabels/MakeLabels, returns them as-is.
	             Otherwise returns MetricZ_LabelUtils::MakeLabels() (base labels only).
	             The result always includes surrounding braces and is never empty.
	    \return string Prometheus label block, e.g. "{world=\"...\",host=\"...\",instance_id=\"...\"}".
	*/
	string GetLabels()
	{
		if (m_Labels == string.Empty)
			return MetricZ_LabelUtils.MakeLabels();

		return m_Labels;
	}

	/**
	    \brief Convert enum type to Prometheus text.
	    \return "gauge" or "counter"; falls back to "gauge" on error
	*/
	private string TypeToText()
	{
		switch (m_Type) {
		case MetricZ_MetricType.GAUGE:
			return "gauge";

		case MetricZ_MetricType.COUNTER:
			return "counter";
		}

		ErrorEx("Invalid metric type " + m_Type.ToString() + " for " + m_Name);
		return "gauge";
	}

	/**
	    \brief Write HELP and TYPE headers.
	    \param fh Open file handle
	*/
	void WriteHeaders(FileHandle fh)
	{
		if (!fh)
			return;

		FPrint(fh, "# HELP " + m_Name + " " + m_Help + "\n");
		FPrint(fh, "# TYPE " + m_Name + " " + TypeToText() + "\n");
	}

	/**
	    \brief Write metric value.
	    \param fh Open file handle
	    \param labels Optional preformatted label set "{k=\"v\"}"
	*/
	void Flush(FileHandle fh, string labels = "")
	{
		if (!fh)
			return;

		if (labels == string.Empty)
			labels = GetLabels();

		FPrint(fh, m_Name + labels + " 0\n");
	}

	/**
	    \brief Write headers then value.
	    \param fh Open file handle
	    \param labels Optional preformatted label set
	*/
	void FlushWithHead(FileHandle fh, string labels = "")
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
