/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Composite Sink pattern.
    \details Forwards metric lines to multiple registered sinks (e.g., File and REST simultaneously).
             Note: Not recommended for production due to double overhead.
*/
class MetricZ_CompositeSink : MetricZ_SinkBase
{
	private ref array<ref MetricZ_SinkBase> m_Sinks;

	void MetricZ_CompositeSink()
	{
		m_Sinks = new array<ref MetricZ_SinkBase>();
	}

	/**
	    \brief Adds a sink to the composite list.
	*/
	void AddSink(MetricZ_SinkBase sink, int bufferLimit)
	{
		if (sink) {
			sink.SetBuffer(bufferLimit);
			m_Sinks.Insert(sink);
		}
	}

	override bool Begin()
	{
		if (!MetricZ_Config.IsLoaded())
			return false;

		if (!super.Begin())
			return false;

		bool anyStarted = false;
		foreach (MetricZ_SinkBase sink : m_Sinks) {
			if (sink.Begin())
				anyStarted = true;
		}

		return anyStarted;
	}

	override void Line(string line)
	{
		if (!IsBusy())
			return;

		foreach (MetricZ_SinkBase sink : m_Sinks)
			sink.Line(line);
	}

	override bool End()
	{
		if (!IsBusy())
			return false;

		bool allEnded = true;

		foreach (MetricZ_SinkBase sink : m_Sinks) {
			if (!sink.End())
				allEnded = false;
		}

		super.End();

		return allEnded;
	}
}
#endif
