/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
typedef map<int, ref array<string>> MetricZ_Cache;

enum MetricZ_CacheKey {
	NONE,
	WEAPON_TYPES,
	AMMO_TYPES,
}

/**
    \brief Global registry for known metric keys persistence.
    \details Stores sets of known labels (e.g. all seen weapon types) to initialize
             counters with 0 on server restart, fixing Prometheus rate calculations.
*/
class MetricZ_PersistentCache {
	protected static bool s_HasChanges;
	protected static ref MetricZ_Cache s_Cache;
	protected static ref JsonSerializer m_Serializer;

	/**
	    \brief Load cache from disk.
	*/
	static bool Load()
	{
		if (!s_Cache)
			s_Cache = new MetricZ_Cache();

		if (!FileExist(MetricZ_Config.METRICS_CACHE))
			return false;

		FileHandle fh = OpenFile(MetricZ_Config.METRICS_CACHE, FileMode.READ);
		if (fh == 0) {
			ErrorEx("MetricZ fail open labels cache file: " + MetricZ_Config.METRICS_CACHE, ErrorExSeverity.ERROR);
			return false;
		}

		string data;

		ReadFile(fh, data, 10485760);
		CloseFile(fh);

		if (!m_Serializer)
			m_Serializer = new JsonSerializer();

		string error;
		if (!m_Serializer.ReadFromString(s_Cache, data, error)) {
			ErrorEx("MetricZ fail load labels cache with error: " + error, ErrorExSeverity.ERROR);
			return false;
		}

#ifdef DIAG
		ErrorEx("MetricZ labels cache loaded from file: " + MetricZ_Config.METRICS_CACHE, ErrorExSeverity.INFO);
#endif

		return true;
	}

	/**
	    \brief Save to disk only if data changed.
	*/
	static bool Save()
	{
		// string filename, T data, out string errorMessage
		if (!s_Cache || !s_HasChanges)
			return false;

		if (!m_Serializer)
			m_Serializer = new JsonSerializer();

		string data;
		if (!m_Serializer.WriteToString(s_Cache, false, data)) {
			ErrorEx("MetricZ fail save labels cache in file: serialization error", ErrorExSeverity.ERROR);
			return false;
		}

		FileHandle fh = OpenFile(MetricZ_Config.METRICS_CACHE, FileMode.WRITE);
		if (fh == 0) {
			ErrorEx("MetricZ fail save labels cache in file: " + MetricZ_Config.METRICS_CACHE, ErrorExSeverity.ERROR);
			return false;
		}

		FPrint(fh, data);
		CloseFile(fh);

		s_HasChanges = false;

#ifdef DIAG
		ErrorEx("MetricZ labels cache saved in file: " + MetricZ_Config.METRICS_CACHE, ErrorExSeverity.INFO);
#endif

		return true;
	}

	/**
	    \brief Register a seen key.
	    \return true if key was new (added), false if already known.
	*/
	static bool Register(MetricZ_CacheKey category, string key)
	{
		if (!s_Cache || category <= MetricZ_CacheKey.NONE || key == string.Empty)
			return false;

		array<string> keys;
		if (!s_Cache.Find(category, keys)) {
			keys = new array<string>();
			s_Cache.Insert(category, keys);
		}

		// If key unknown -> add and mark dirty
		if (keys.Find(key) == -1) {
			keys.Insert(key);
			s_HasChanges = true;
			return true;
		}

		return false;
	}

	/**
	    \brief Get all known keys for a category to initialize counters.
	*/
	static array<string> GetKeys(MetricZ_CacheKey category)
	{
		if (!s_Cache)
			return null;

		return s_Cache.Get(category);
	}
}
#endif
