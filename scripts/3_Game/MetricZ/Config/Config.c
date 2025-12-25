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
	protected static bool s_TelemetrySend;

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
		if (s_Config.file.enabled)
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

				ErrorEx(
				    string.Format("MetricZ: saved upgraded config file: %1", cfgFile),
				    ErrorExSeverity.INFO);
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

		ErrorEx(
		    string.Format("MetricZ: saved new config file: %1", cfgFile),
		    ErrorExSeverity.INFO);
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

			string msg = "MetricZ: Legacy export path detected!\n\n";
			msg += "========================================= LEGACY WARNING =========================================\n";
			msg += "| FILE:      '" + MetricZ_Constants.LEGACY_PROM_FILE + "'\n";
			msg += "| REQUIRED:  Please delete this file to enable the new 'export/' directory structure.\n";
			msg += "| NOTE:      You also need to update the path in the exporter file collector to the new destination.\n";
			msg += "| WARNING:   Support for legacy paths will be removed in future releases.\n";
			msg += "==================================================================================================\n\n";
			ErrorEx(msg, ErrorExSeverity.INFO);

			return;
		}

		string exportDir = MetricZ_Constants.EXPORT_DIR;
		if (!FileExist(exportDir))
			MakeDirectory(exportDir);

		string fileName = s_Config.file.file_name;
		if (fileName == string.Empty)
			fileName = string.Format("metricz_%1", s_Config.settings.instance_id_resolved);

		s_Config.file.prom_file_path = string.Format("%1%2.prom", exportDir, fileName);
		s_Config.file.temp_file_path = string.Format("%1%2.tmp", exportDir, fileName);
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
#else
		if (!s_Config.settings.disable_telemetry && !s_TelemetrySend) {
			s_TelemetrySend = true;
			int delay = Math.RandomInt(MetricZ_Constants.TELEMETRY_DELAY, MetricZ_Constants.TELEMETRY_DELAY * 2);
			g_Game.GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SendTelemetry, delay, false);
		}
#endif

		string msg = string.Format(
		                 "MetricZ: loaded %1 (%2) build %3, instance_id=%4",
		                 MetricZ_Constants.VERSION,
		                 MetricZ_Constants.COMMIT_SHA,
		                 MetricZ_Constants.BUILD_DATE,
		                 s_Config.settings.instance_id_resolved);

		ErrorEx(msg, ErrorExSeverity.INFO);
	}

	/**
	    \brief Telemetry sender
	*/
	protected void SendTelemetry()
	{
		RestApi api = GetRestApi();
		if (!api)
			api = CreateRestApi();
		if (!api) {
			s_TelemetrySend = false;
			return;
		}

		RestContext ctx = api.GetRestContext(MetricZ_Constants.TELEMETRY_URL);
		if (!ctx) {
			s_TelemetrySend = false;
			return;
		}

		string body = string.Format(
		                  "{\"application\":\"%1\",\"version\":\"%2\",\"type\":\"steam\",\"port\":%3}",
		                  MetricZ_Constants.NAME,
		                  MetricZ_Constants.VERSION,
		                  g_Game.ServerConfigGetInt("steamQueryPort"));

		ctx.SetHeader("application/json");
		ctx.POST(null, "/api/telemetry", body);
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
