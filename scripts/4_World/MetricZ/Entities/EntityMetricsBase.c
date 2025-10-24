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
	    \brief Flush all registered metrics.
	    \param fh Open file handle
	*/
	void Flush(FileHandle fh)
	{
		if (!fh || m_Registry.Count() < 1)
			return;

		foreach (MetricZ_MetricBase m : m_Registry)
			m.Flush(fh, LabelsFor(m));
	}

	/**
	    \brief Flush single metric by index.
	    \param fh Open file handle
	    \param idx Metric index in registry
	*/
	void FlushAt(FileHandle fh, int idx)
	{
		if (!fh || idx < 0 || idx >= m_Registry.Count())
			return;

		m_Registry[idx].Flush(fh, LabelsFor(m_Registry[idx]));
	}

	/**
	    \brief Write HELP/TYPE for all metrics.
	    \param fh Open file handle
	*/
	void WriteHeaders(FileHandle fh)
	{
		if (!fh || m_Registry.Count() < 1)
			return;

		foreach (MetricZ_MetricBase m : m_Registry)
			m.WriteHeaders(fh);
	}

	/**
	    \brief Write HELP/TYPE for one metric by index.
	    \param fh Open file handle
	    \param idx Metric index in registry
	*/
	void WriteHeaderAt(FileHandle fh, int idx)
	{
		if (!fh || idx < 0 || idx >= m_Registry.Count())
			return;

		m_Registry[idx].WriteHeaders(fh);
	}

	/**
	    \brief Select a set of labels for a specific metric.
	    \details Defaults to the generic m_Labels. Override in descendants as needed.
	*/
	protected void SetLabels()
	{
		if (m_Labels != string.Empty)
			return;

		m_Labels = MetricZ_LabelUtils.MakeLabels(null);
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
