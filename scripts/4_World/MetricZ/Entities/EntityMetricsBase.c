/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Database for entity metrics collectors.
    \details Maintains the metrics registry and common Count/Flush/FlushAt/WriteHeaders/WriteHeaderAt code.
*/
class MetricZ_EntityMetricsBase
{
	protected string m_Labels;
	protected ref array<ref MetricZ_MetricBase> m_Registry = new array<ref MetricZ_MetricBase>();

	/**
	    \brief Return number of registered metrics.
	    \return \p int Count of metrics
	*/
	int Count()
	{
		return m_Registry.Count();
	}

	/**
	    \brief Update all metrics from the entity state.
	*/
	void Update() {} // redefined in children

	/**
	    \brief Get metric object directly by index (Unsafe/Fast).
	*/
	MetricZ_MetricBase GetMetricDirect(int idx)
	{
		return m_Registry.Get(idx);
	}

	/**
	    \brief Get cached labels string directly (Unsafe/Fast).
	*/
	string GetLabelsDirect()
	{
		return m_Labels;
	}

	/**
	    \brief Flush all registered metrics.
	    \param MetricZ_SinkBase sink instance
	*/
	void Flush(MetricZ_SinkBase sink)
	{
		if (!sink || m_Registry.Count() < 1)
			return;

		foreach (MetricZ_MetricBase metric : m_Registry)
			metric.Flush(sink, LabelsFor(metric));
	}

	/**
	    \brief Flush single metric by index.
	    \param MetricZ_SinkBase sink instance
	    \param idx Metric index in registry
	*/
	void FlushAt(MetricZ_SinkBase sink, int idx)
	{
		if (!sink || idx < 0 || idx >= m_Registry.Count())
			return;

		m_Registry[idx].Flush(sink, LabelsFor(m_Registry[idx]));
	}

	/**
	    \brief Write HELP/TYPE for all metrics.
	    \param MetricZ_SinkBase sink instance
	*/
	void WriteHeaders(MetricZ_SinkBase sink)
	{
		if (!sink || m_Registry.Count() < 1)
			return;

		foreach (MetricZ_MetricBase metric : m_Registry)
			metric.WriteHeaders(sink);
	}

	/**
	    \brief Write HELP/TYPE for one metric by index.
	    \param MetricZ_SinkBase sink instance
	    \param idx Metric index in registry
	*/
	void WriteHeaderAt(MetricZ_SinkBase sink, int idx)
	{
		if (!sink || idx < 0 || idx >= m_Registry.Count())
			return;

		m_Registry[idx].WriteHeaders(sink);
	}

	/**
	    \brief Select a set of labels for a specific metric.
	    \details Defaults to the generic m_Labels. Override in descendants as needed.
	*/
	protected void SetLabels()
	{
		if (m_Labels == string.Empty)
			m_Labels = MetricZ_LabelUtils.MakeLabels(null);

		ApplyLabelsToRegistry();
	}

	/**
	    \brief Set labels direct to metric with execute SetLabels() for each metric in registry
	*/
	protected void ApplyLabelsToRegistry()
	{
		if (m_Registry.Count() == 0)
			return;

		foreach (MetricZ_MetricBase metric : m_Registry)
			metric.SetLabels(m_Labels);
	}

	/**
	    \brief Select a set of labels for a specific metric.
	    \details Defaults to the generic m_Labels. Override in descendants as needed.
	*/
	protected string LabelsFor(MetricZ_MetricBase metric)
	{
		return m_Labels;
	}
}
#endif
