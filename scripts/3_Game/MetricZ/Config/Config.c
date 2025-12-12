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
		ErrorEx("MetricZ: configuration reset", ErrorExSeverity.INFO);
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
		LoadConfigFile();

		InitFileExport();

		// Init geo cache
		MetricZ_Geo.Init();

		// Load labels cache
		MetricZ_PersistentCache.Load();
	}

	/**
	    \brief Load, create or update JSON configuration file
	*/
	protected void LoadConfigFile()
	{
		if (!FileExist(MetricZ_Constants.WORK_DIR))
			MakeDirectory(MetricZ_Constants.WORK_DIR);

		string cfgFile = MetricZ_Constants.CONFIG_FILE;
		string error;
		if (FileExist(cfgFile)) {
			if (!JsonFileLoader<MetricZ_ConfigDTO>.LoadFile(cfgFile, s_Config, error)) {
				ErrorEx(error);
				return;
			}

			s_Config.Normalize();

			if (s_Config.version != MetricZ_Constants.VERSION) {
				s_Config.version = MetricZ_Constants.VERSION;

				if (!JsonFileLoader<MetricZ_ConfigDTO>.SaveFile(cfgFile, s_Config, error)) {
					ErrorEx(error);
					return;
				}

				ErrorEx("MetricZ: saved upgraded config file: " + cfgFile, ErrorExSeverity.INFO);
			}

			SetLoaded();
			return;
		}

		// Write initial or upgraded config
		s_Config.version = MetricZ_Constants.VERSION;

		if (!JsonFileLoader<MetricZ_ConfigDTO>.SaveFile(cfgFile, s_Config, error)) {
			ErrorEx(error);
			return;
		}

		ErrorEx("MetricZ: saved new config file: " + cfgFile, ErrorExSeverity.INFO);
		SetLoaded();
	}

	/**
	    \brief Initialize path for file export with legacy file path support
	*/
	protected void InitFileExport()
	{
		if (FileExist(MetricZ_Constants.LEGACY_PROM_FILE)) {
			s_Config.file.prom_file_path = MetricZ_Constants.LEGACY_PROM_FILE;
			s_Config.file.temp_file_path = MetricZ_Constants.LEGACY_TMP_FILE;

			string msg = "MetricZ: Legacy export path detected!\n";
			msg += "File found at: '" + MetricZ_Constants.LEGACY_PROM_FILE + "'\n";
			msg += "ACTION REQUIRED: Please delete this file to enable the new 'export/' directory structure.\n";
			msg += "NOTE: You must also update your external metrics collector path.\n";
			msg += "WARNING: Support for legacy paths will be removed in future releases.\n";
			ErrorEx(msg, ErrorExSeverity.WARNING);

			return;
		}

		string exportDir = MetricZ_Constants.EXPORT_DIR;
		if (s_Config.file.enabled && !FileExist(exportDir))
			MakeDirectory(exportDir);

		string fileName = s_Config.file.file_name;
		if (fileName == string.Empty)
			fileName = "metricz_" + s_Config.settings.instance_id;

		s_Config.file.prom_file_path = exportDir + fileName + ".prom";
		s_Config.file.temp_file_path = exportDir + fileName + ".tmp";
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

		string ver = string.Format(
		                 "%1 (%2) build %3",
		                 MetricZ_Constants.VERSION,
		                 MetricZ_Constants.COMMIT_SHA,
		                 MetricZ_Constants.BUILD_DATE);

		ErrorEx("MetricZ: loaded " + ver, ErrorExSeverity.INFO);
	}

	protected void DebugConfig()
	{
		string json;
		JsonSerializer().WriteToString(s_Config, true, json);
		ErrorEx("MetricZ: configuration trace:\n" + json, ErrorExSeverity.INFO);
	}
}
#endif
