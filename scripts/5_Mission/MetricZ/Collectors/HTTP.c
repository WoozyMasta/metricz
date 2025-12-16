/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/logz
*/

#ifdef SERVER
/**
    \brief Base class for MetricZ collectors.
*/
class MetricZ_CollectorHTTP : MetricZ_CollectorBase
{
	override string GetName()
	{
		return "http";
	}

	override bool IsEnabled()
	{
		if (!MetricZ_Config.IsLoaded())
			return false;

		MetricZ_ConfigDTO cfg = MetricZ_Config.Get();
		return (!cfg.disabled_metrics.http && cfg.http.enabled);
	}

	override void Flush(MetricZ_SinkBase sink)
	{
		MetricZ_HttpStats.Flush(sink);
	}
}
#endif
