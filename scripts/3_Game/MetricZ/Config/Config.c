/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Configuration Manager for MetricZ.
    \details Handles the lifecycle of the configuration: loading from JSON, version migration,
             creating default files, and initializing export paths.
             Implements the Singleton pattern.
*/
class MetricZ_Config
{
	protected static ref MetricZ_Config s_Instance; // singleton instance
	protected static ref MetricZ_ConfigDTO s_Config; // singleton state
	protected static bool s_Loaded;

	/**
	    \brief Retrieves the singleton configuration instance.
	    \details Lazy-loads the configuration file on the first call.
	    \return Global MetricZ_ConfigDTO instance.
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
	    \brief Checks if the configuration is successfully loaded.
	*/
	static bool IsLoaded()
	{
		return (s_Instance && s_Config && s_Loaded);
	}

	/**
	    \brief Orchestrates the loading process.
	*/
	protected void Load()
	{
		// Read or create/update JSON config
		LoadConfigFile();

		// Setup export paths
		InitFileExport();

		// Init geo cache
		MetricZ_Geo.Init();

		// Load labels cache
		MetricZ_PersistentCache.Load();
	}

	/**
	    \brief Handles JSON file operations (Load/Save/Upgrade).
	    \details If the file exists, it attempts to load and validate it.
	             If the version mismatches, it performs an upgrade and saves the file.
	             If the file is missing, it creates a new default configuration.
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
	    \brief Determines the file paths for metric export.
	*/
	protected void InitFileExport()
	{
		if (FileExist(MetricZ_Constants.LEGACY_PROM_FILE)) {
			s_Config.file.prom_file_path = MetricZ_Constants.LEGACY_PROM_FILE;
			s_Config.file.temp_file_path = MetricZ_Constants.LEGACY_TMP_FILE;

			string msg = "MetricZ: Legacy export path detected!\n";
			msg += "FILE:      '" + MetricZ_Constants.LEGACY_PROM_FILE + "'\n\n";
			msg += "REQUIRED:  Please delete this file to enable the new 'export/' directory structure.\n";
			msg += "NOTE:      You must also update your external metrics collector path.\n";
			msg += "WARNING:   Support for legacy paths will be removed in future releases.\n";
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
	    \brief Finalizes the loading process.
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

	/**
	    \brief Dumps the current configuration to the script log
	*/
	protected void DebugConfig()
	{
		string json;
		JsonSerializer().WriteToString(s_Config, true, json);
		ErrorEx("MetricZ: configuration trace:\n" + json, ErrorExSeverity.INFO);
	}
}
#endif
