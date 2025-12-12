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
	protected string m_Type;
	protected string m_Labels;
	protected MetricZ_MetricType m_EType;

	/**
	    \brief Constructor.
	    \param name Metric name
	    \param help HELP text
	    \param type Metric type (GAUGE/COUNTER)
	*/
	void MetricZ_MetricBase(string name, string help, MetricZ_MetricType type = 0)
	{
		m_Name = MetricZ_Constants.NAMESPACE + name;
		if (type == MetricZ_MetricType.COUNTER)
			m_Name += "_total";

		m_EType = type;
		m_Help = "# HELP " + m_Name + " " + help;
		m_Type = "# TYPE " + m_Name + " " + TypeToText();
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
	    \brief Build and set labels from a single key/value pair.
	    \details
	      Equivalent to MakeLabels(...) with a one-element map:
	        { key = value } plus base labels {world,host,instance_id}.
	    \param key   Label key (e.g. "weapon")
	    \param value Label value (will be escaped inside MakeLabels)
	*/
	void MakeLabel(string key, string value)
	{
		map<string, string> labels = new map<string, string>();
		labels.Insert(key, value);
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
		return m_Help;
	}

	/**
	    \brief Build TYPE header.
	    \return \p string "# TYPE ..."
	*/
	string GetType()
	{
		return m_Type;
	}

	/**
	    \brief Get metric type enum.
	    \return \p MetricZ_MetricType
	*/
	MetricZ_MetricType GetMetricType()
	{
		return m_EType;
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
	    \brief Get raw labels for this metric, unchanged and without presetting default values.
	        \return string Prometheus label block, e.g. "{world=\"...\",...}" or empty string.
	*/
	string GetLabelsRaw()
	{
		return m_Labels;
	}

	/**
	    \brief Checks whether there is an explicitly defined label for the given metric.
	        \return bool true if there is a label.
	*/
	bool HasLabels()
	{
		return (m_Labels != string.Empty);
	}

	/**
	    \brief Convert enum type to Prometheus text.
	    \return "gauge" or "counter"; falls back to "gauge" on error
	*/
	private string TypeToText()
	{
		switch (m_EType) {
		case MetricZ_MetricType.GAUGE:
			return "gauge";

		case MetricZ_MetricType.COUNTER:
			return "counter";
		}

		ErrorEx("MetricZ: invalid metric type " + m_EType.ToString() + " for " + m_Name);
		return "gauge";
	}

	/**
	    \brief Write HELP and TYPE headers.
	    \param MetricZ_SinkBase sink instance
	*/
	void WriteHeaders(MetricZ_SinkBase sink)
	{
		if (!sink)
			return;

		sink.Line(m_Help);
		sink.Line(m_Type);
	}

	/**
	    \brief Write metric value.
	    \param MetricZ_SinkBase sink instance
	    \param labels Optional preformatted label set "{k=\"v\"}"
	*/
	void Flush(MetricZ_SinkBase sink, string labels = "")
	{
		if (!sink)
			return;

		if (labels == string.Empty)
			labels = GetLabels();

		sink.Line(m_Name + labels + " 0");
	}

	/**
	    \brief Write headers then value.
	    \param MetricZ_SinkBase sink instance
	    \param labels Optional preformatted label set
	*/
	void FlushWithHead(MetricZ_SinkBase sink, string labels = "")
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
