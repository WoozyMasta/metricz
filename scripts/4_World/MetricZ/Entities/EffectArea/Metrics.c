/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Per-EffectArea metrics collector.
    \details Caches metric instances and label sets. Updated during scrape.
*/
class MetricZ_EffectAreaMetrics : MetricZ_EntityMetricsBase
{
	protected EffectArea m_Area; //!< Parent EffectArea instance.
	ref MetricZ_MetricFloat m_Radius; //!< Metric: Radius of the area.
	ref MetricZ_MetricInt m_Insiders; //!< Metric: Count of players inside the area.

	/**
	    \brief Constructor. Initializes metric instances and spawn tick.
	*/
	void MetricZ_EffectAreaMetrics()
	{
		m_Insiders = new MetricZ_MetricInt(
		    "effect_area_insiders",
		    "Count of players inside Effect Area",
		    MetricZ_MetricType.GAUGE);
	}

	/**
	    \brief One-time registry fill.
	    \param area EffectArea instance to initialize metrics for.
	*/
	void Init(EffectArea area)
	{
		if (!area || m_Registry.Count() > 0)
			return;

		m_Area = area;

		m_Registry.Insert(m_Insiders);

		SetLabels();
	}

	/**
	    \brief Update all metrics from the EffectArea state.
	*/
	override void Update()
	{
		if (!m_Area || m_Registry.Count() < 1)
			return;

		m_Insiders.Set(m_Area.MetricZ_GetInsidersCount());
	}

	/**
	    \brief Build and cache label sets using integer coordinates.
	*/
	override protected void SetLabels()
	{
		if (!m_Area || m_Labels != string.Empty)
			return;

		vector pos = m_Area.m_Position;
		if (pos == vector.Zero)
			pos = m_Area.GetPosition();

		float lon, lat;
		MetricZ_Geo.GetLonLat(pos, lon, lat);

		map<string, string> labels = new map<string, string>();
		labels.Insert("type", m_Area.MetricZ_GetType());
		labels.Insert("class", m_Area.ClassName());
		labels.Insert("longitude", lon.ToString());
		labels.Insert("latitude", lat.ToString());
		labels.Insert("radius", m_Area.MetricZ_GetRadius().ToString());

		m_Labels = MetricZ_LabelUtils.MakeLabels(labels);

		ApplyLabelsToRegistry();
	}
}
#endif
