/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/logz
*/

#ifdef SERVER
/**
    \brief Base class for MetricZ collectors.
*/
class MetricZ_CollectorBase
{
	/**
	    \brief Component name of metrics bucket.
	*/
	string GetName()
	{
		return "unknown";
	}

	/**
	    \brief Check if this collector is enabled.
	*/
	bool IsEnabled()
	{
		return MetricZ_Config.IsLoaded();
	}

	/**
	    \brief Main flush method called by MetricZ.
	*/
	void Flush(MetricZ_SinkBase sink)
	{
		// override in subclasses
	}
}
#endif
