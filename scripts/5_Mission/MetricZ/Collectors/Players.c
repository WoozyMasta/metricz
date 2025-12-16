/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/logz
*/

#ifdef SERVER
/**
    \brief Base class for MetricZ collectors.
*/
class MetricZ_CollectorPlayers : MetricZ_CollectorBase
{
	override string GetName()
	{
		return "players";
	}

	override bool IsEnabled()
	{
		return (MetricZ_Config.IsLoaded() && !MetricZ_Config.Get().disabled_metrics.players);
	}

	override void Flush(MetricZ_SinkBase sink)
	{
		MetricZ_EntitiesWriter.FlushPlayers(sink);
	}
}
#endif
