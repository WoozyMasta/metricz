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
	// Build information
	static const string VERSION = "0.3.0";
	static const string COMMIT_SHA = "fd8bfa6";
	static const string BUILD_DATE = "2025-12-12T10:13:25+00:00";

	// Default buffer size limit
	static const int MAX_BUFFER_SIZE = 65536;

	// Metrics prefix
	static const string NAMESPACE = "dayz_metricz_";

	// Working directory
	static const string WORK_DIR = "$profile:metricz/";
	static const string CONFIG_FILE = WORK_DIR + "config.json";
	static const string CACHE_FILE = WORK_DIR + "cache.json";

	// File export directory
	static const string EXPORT_DIR = WORK_DIR + "export/";

	// Legacy files support
	static const string LEGACY_PROM_FILE = "$profile:metricz.prom";
	static const string LEGACY_TMP_FILE = "$profile:metricz.tmp";
}
#endif
