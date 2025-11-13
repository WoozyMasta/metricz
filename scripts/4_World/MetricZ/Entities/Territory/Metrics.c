/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Per-TerritoryFlag metrics collector.
    \details Caches metric instances and label sets. Updated during scrape.
*/
class MetricZ_TerritoryMetrics : MetricZ_EntityMetricsBase
{
	// parent TerritoryFlag
	protected TerritoryFlag m_TerritoryFlag;

	// generic
	ref MetricZ_MetricFloat m_Lifetime;

	/**
	    \brief Constructor. Initializes metric instances and spawn tick.
	*/
	void MetricZ_TerritoryMetrics()
	{
		m_Lifetime = new MetricZ_MetricFloat(
		    "territory_lifetime",
		    "Territory flag lifetime fraction 0..1",
		    MetricZ_MetricType.GAUGE);
	}

	/**
	    \brief One-time registry fill.
	*/
	void Init(TerritoryFlag territory)
	{
		if (!territory || m_Registry.Count() > 0)
			return;

		m_TerritoryFlag = territory;

		m_Registry.Insert(m_Lifetime);

		SetLabels();
	}

	/**
	    \brief Update all metrics from the territory state.
	*/
	override void Update()
	{
		if (!m_TerritoryFlag || m_Registry.Count() < 1)
			return;

		m_Lifetime.Set(m_TerritoryFlag.GetRefresherTime01());
	}

	/**
	    \brief Build and cache territory label sets.
	*/
	override protected void SetLabels()
	{
		if (!m_TerritoryFlag || m_Labels != string.Empty)
			return;

		// position as int for drop noise after point
		vector p = m_TerritoryFlag.GetPosition();
		int x = Math.Round(p[0]);
		int y = Math.Round(p[1]);
		int z = Math.Round(p[2]);

		map<string, string> labels = new map<string, string>();
		labels.Insert("x", x.ToString());
		labels.Insert("y", y.ToString());
		labels.Insert("z", z.ToString());

		m_Labels = MetricZ_LabelUtils.MakeLabels(labels);
	}
}
#endif
