/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Runtime and CLI configuration for MetricZ.
    \details Reads overrides from serverDZ.cfg via ServerConfigGetInt and from CLI via GetCLIParam.
*/
class MetricZ_Config
{
	// singleton
	protected static ref MetricZ_Config s_Instance;
	protected static ref MetricZ_ConfigDTO s_Config;
	protected static bool s_Loaded;

	/**
	    \brief Get singleton instance (lazy-loads file once).
	*/
	static MetricZ_ConfigDTO Get()
	{
		if (!s_Instance)
			s_Instance = new MetricZ_Config();

		if (!s_Config && !s_Loaded) {
			s_Config = new MetricZ_ConfigDTO();
			s_Instance.Load();
		}

		return s_Config;
	}

	/**
	    \brief Drop instance and force reload on next
	*/
	static void Reset()
	{
		if (s_Config)
			s_Config = null;

		s_Loaded = false;
		ErrorEx("MetricZ configuration reset", ErrorExSeverity.INFO);
	}

	/**
	    \brief Check config loaded
	*/
	static bool IsLoaded()
	{
		return (s_Instance && s_Config && s_Loaded);
	}

	/**
	    \brief Load and apply configuration overrides.
	    \details Reads options from serverDZ.cfg (keys prefixed with "MetricZ_")
	            and CLI flags (prefixed with "metricz-"). CLI overrides config.
	            Converts seconds to milliseconds and enforces minimums.
	*/
	protected void Load()
	{
		// Read or create/update JSON config
		LoadOrCreateFile();

		// Init geo cache
		MetricZ_Geo.Init();

		// Load labels cache
		MetricZ_PersistentCache.Load();
	}

	protected void LoadOrCreateFile()
	{
		string cfg = MetricZ_Constants.CONFIG_FILE;

		string error;
		if (FileExist(cfg)) {
			if (!JsonFileLoader<MetricZ_ConfigDTO>.LoadFile(cfg, s_Config, error)) {
				ErrorEx(error);
				return;
			}

			s_Config.Normalize();

			if (s_Config.version != MetricZ_Constants.VERSION) {
				s_Config.version = MetricZ_Constants.VERSION;

				if (!JsonFileLoader<MetricZ_ConfigDTO>.SaveFile(cfg, s_Config, error)) {
					ErrorEx(error);
					return;
				}

				ErrorEx("Saved upgraded MetricZ config file: " + cfg, ErrorExSeverity.INFO);
			}

			SetLoaded();
			return;
		}

		// Write initial or upgraded config
		s_Config.version = MetricZ_Constants.VERSION;

		if (!JsonFileLoader<MetricZ_ConfigDTO>.SaveFile(cfg, s_Config, error)) {
			ErrorEx(error);
			return;
		}

		ErrorEx("Saved new MetricZ config file: " + cfg, ErrorExSeverity.INFO);
		SetLoaded();
	}

	/**
	    \brief Mark config as loaded (for logging and control flow).
	*/
	protected void SetLoaded()
	{
		if (s_Config)
			s_Config.Normalize();

		s_Loaded = true;

#ifdef DIAG
		DebugConfig();
#endif

		ErrorEx("MetricZ loaded", ErrorExSeverity.INFO);
	}

	protected void DebugConfig()
	{
		string json;
		JsonSerializer().WriteToString(s_Config, true, json);
		ErrorEx("MetricZ configuration trace:\n" + json, ErrorExSeverity.INFO);
	}
}
#endif
