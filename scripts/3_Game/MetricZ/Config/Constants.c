/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief MetricZ Constants
*/
class MetricZ_Constants
{
	static const int VERSION = 0;
	static const int MAX_BUFFER_SIZE = 65536;
	static const string CONFIG_FILE = "$profile:metricz.json";
	static const string PROM_FILE = "$profile:metricz.prom";
	static const string TEMP_FILE = "$profile:metricz.tmp";
	static const string CACHE_FILE = "$profile:metricz.cache";
	static const string NAMESPACE = "dayz_metricz_";
}
#endif
