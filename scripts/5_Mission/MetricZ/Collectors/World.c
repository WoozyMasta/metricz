/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/logz
*/

#ifdef SERVER
/**
    \brief Base class for MetricZ collectors.
*/
class MetricZ_CollectorWorld : MetricZ_CollectorBase
{
	override string GetName()
	{
		return "world";
	}

	override bool IsEnabled()
	{
		return MetricZ_Config.IsLoaded();
	}

	override void Flush(MetricZ_SinkBase sink)
	{
		if (!MetricZ_Config.IsLoaded())
			return;

		MetricZ_Storage.Update();
		MetricZ_Storage.Flush(sink);
	}
}
#endif
