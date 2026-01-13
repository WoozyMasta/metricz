/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/logz
*/

#ifdef SERVER
/**
    \brief Collector for Event stats.
*/
class MetricZ_CollectorEvents : MetricZ_CollectorBase
{
	override string GetName()
	{
		return "events";
	}

	override bool IsEnabled()
	{
		return (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disabled_metrics.events);
	}

	override void Flush(MetricZ_SinkBase sink)
	{
		MetricZ_EventStats.Flush(sink);
	}
}
#endif
