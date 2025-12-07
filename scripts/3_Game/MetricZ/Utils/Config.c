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
	// constants
	protected static const int CONFIG_VERSION = 0;
	static const string METRICS_CONFIG = "$profile:metricz.json";
	static const string METRICS_FILE = "$profile:metricz.prom";
	static const string METRICS_TEMP = "$profile:metricz.tmp";
	static const string METRICS_CACHE = "$profile:metricz.cache";
	static const string NS = "dayz_metricz_";

	// singleton
	protected static ref MetricZ_Config s_Instance;
	protected static bool s_Loaded;

	// * [start public options]

	// Internal config version, do not touch it
	int configVersion = CONFIG_VERSION;

	// Delay before the first metric collection in seconds
	int initDelaySeconds = 60; // 0, 300

	// Interval between metric updates in seconds
	int scrapeIntervalSeconds = 15; // 1, 300

	// Disable player-related metrics collection
	// `dayz_metricz_player_*`
	bool disablePlayerMetrics;

	// Disable zombie per-type and mind states metrics collection
	// `dayz_metricz_animals_by_type`
	bool disableZombieMetrics;

	// Disable animal per-type metrics collection
	// `dayz_metricz_infected_by_type` and `dayz_metricz_infected_mind_state`
	bool disableAnimalMetrics;

	// Disable vehicle and transport metrics collection
	// `dayz_metricz_transport_*`
	bool disableTransportMetrics;

	// Disable weapon per-type (count, shoot, kills and hits) metrics collection
	// `dayz_metricz_weapon_shots_total`, `dayz_metricz_weapons_by_type_total`,
	// `dayz_metricz_player_killed_by_total` and `dayz_metricz_creature_killed_by_total`
	bool disableWeaponMetrics;

	// Disable player and zombie/animal hit by ammo type metrics
	// `dayz_metricz_player_killed_by_total` and `dayz_metricz_creature_killed_by_total`
	bool disableEntityHitsMetrics;

	// Minimum damage to log in `EEHitBy()`;
	// -1 and less disables threshold.
	float entityHitDamageThreshold = 3; // -1, 100

	// Minimum vehicle-hit damage to log in `EEHitBy()`;
	// -1 and less disables threshold.
	float entityVehicleHitDamageThreshold = 15; // -1, 100

	// Disable player and zombie/animal kill by object killer metrics
	bool disableEntityKillsMetrics;

	// Disable territory flag metrics collection
	bool disableTerritoryMetrics;

	// Disable EffectArea (Contaminated, Geyser, HotSpring, Volcanic, etc.) metrics
	bool disableEffectAreaMetrics;

	// Enable Local EffectArea metrics like ContaminatedArea_Local created from Grenade_ChemGas
	// This is disabled by default because metrics for such local zones will always have unique
	// positions, thereby creating new metric series in the TSDB each time.
	// Use with caution, as this may bloat your metrics database!
	bool enableLocalEffectAreaMetrics;

	// Disable player and transport coordinate metrics
	bool disableCoordinatesMetrics;

	// Disable conversion of coordinates metrics to geo `EPSG:4326` (WGS84) format.
	// By default convert position to lon/lat in `-180/180` and `-90/90` range.
	// If disable, all exported coordinates hold vanilla zero relative meters
	bool disableGeoCoordinatesFormat;

	// Disable RPC metrics collection
	// `dayz_metricz_rpc_input_total`
	bool disableRPCMetrics;

	// Disable event handler metrics collection
	// `dayz_metricz_events_total`
	bool disableEventMetrics;

	// Override effective map tiles size in world units.
	// Useful if the web map size is larger than the game world size
	// (for example, the izurvive tiles for Chernarus have a size of `15926`, although the world size is `15360`)
	float mapEffectiveSize; // g_Game.GetWorld().GetWorldSize(), 81920

	// * [end public options]

	// server params -> metrics
	[NonSerialized()]
	int maxPlayers = 255; // from serverDZ.cfg:maxPlayers
	[NonSerialized()]
	int limitFPS = 0; // from CLI:-limitFPS

	/**
	    \brief Get singleton instance (lazy-loads file once).
	*/
	static MetricZ_Config Get()
	{
		if (!s_Instance)
			s_Instance = new MetricZ_Config();

		if (!s_Loaded)
			s_Instance.Load();

		return s_Instance;
	}

	/**
	    \brief Drop instance and force reload on next
	*/
	static void Reset()
	{
		if (s_Instance)
			s_Instance = null;

		s_Loaded = false;
		ErrorEx("MetricZ configuration reset", ErrorExSeverity.INFO);
	}

	/**
	    \brief Check config loaded
	*/
	static bool IsLoaded()
	{
		return s_Loaded;
	}

	/**
	    \brief Load and apply configuration overrides.
	    \details Reads options from serverDZ.cfg (keys prefixed with "MetricZ_")
	            and CLI flags (prefixed with "metricz-"). CLI overrides config.
	            Converts seconds to milliseconds and enforces minimums.
	*/
	protected void Load()
	{
		// server params
		int v = g_Game.ServerConfigGetInt("maxPlayers");
		if (v > 0)
			maxPlayers = v;

		string flagValue;
		if (GetCLIParam("limitFPS", flagValue)) {
			int fps = flagValue.ToInt();
			if (fps > 0)
				limitFPS = fps;
		}

		// Read or create/update JSON config
		LoadOrCreateFile();

		// Init geo cache
		MetricZ_Geo.Init();

		// Load labels cache
		MetricZ_PersistentCache.Load();
	}

	protected void LoadOrCreateFile()
	{
		string error;
		if (FileExist(METRICS_CONFIG)) {
			if (!JsonFileLoader<MetricZ_Config>.LoadFile(METRICS_CONFIG, this, error)) {
				ErrorEx(error);
				return;
			}

			Normalize();

			if (configVersion != CONFIG_VERSION) {
				configVersion = CONFIG_VERSION;

				if (!JsonFileLoader<MetricZ_Config>.SaveFile(METRICS_CONFIG, this, error)) {
					ErrorEx(error);
					return;
				}

				ErrorEx("Saved upgraded MetricZ config file: " + METRICS_CONFIG, ErrorExSeverity.INFO);
			}

			SetLoaded();
			return;
		}

		// Write initial or upgraded config
		configVersion = CONFIG_VERSION;

		if (!JsonFileLoader<MetricZ_Config>.SaveFile(METRICS_CONFIG, this, error)) {
			ErrorEx(error);
			return;
		}

		ErrorEx("Saved new MetricZ config file: " + METRICS_CONFIG, ErrorExSeverity.INFO);
		SetLoaded();
	}

	/**
	    \brief Mark config as loaded (for logging and control flow).
	*/
	protected void SetLoaded()
	{
		s_Loaded = true;

#ifdef DIAG
		DebugConfig();
#endif

		ErrorEx("MetricZ loaded", ErrorExSeverity.INFO);
	}

	/**
	    \brief Clamp user-editable values to safe ranges and derive cached fields.
	*/
	protected void Normalize()
	{
		// Core timing & jitter
		initDelaySeconds = Math.Clamp(initDelaySeconds, 0, 300);
		scrapeIntervalSeconds = Math.Clamp(scrapeIntervalSeconds, 1, 100);
		entityHitDamageThreshold = Math.Clamp(entityHitDamageThreshold, -1, 100);
		entityVehicleHitDamageThreshold = Math.Clamp(entityVehicleHitDamageThreshold, -15, 100);
		mapEffectiveSize = Math.Clamp(mapEffectiveSize, g_Game.GetWorld().GetWorldSize(), 81920);
	}

	protected void DebugConfig()
	{
		string log = "MetricZ configuration trace:\n";

		log += "  Loaded: " + s_Loaded + "\n";
		log += "  InitDelayMs: " + initDelaySeconds + "\n";
		log += "  ScrapeIntervalMs: " + scrapeIntervalSeconds + "\n";
		log += "  DisablePlayerMetrics: " + disablePlayerMetrics + "\n";
		log += "  DisableZombieMetrics: " + disableZombieMetrics + "\n";
		log += "  DisableAnimalMetrics: " + disableAnimalMetrics + "\n";
		log += "  DisableTransportMetrics: " + disableTransportMetrics + "\n";
		log += "  DisableWeaponMetrics: " + disableWeaponMetrics + "\n";
		log += "  DisableEntityHitsMetrics: " + disableEntityHitsMetrics + "\n";
		log += "  EntityHitDamageThreshold: " + entityHitDamageThreshold + "\n";
		log += "  EntityVehicleHitDamageThreshold: " + entityVehicleHitDamageThreshold + "\n";
		log += "  DisableEntityKillsMetrics: " + disableEntityKillsMetrics + "\n";
		log += "  DisableTerritoryMetrics: " + disableTerritoryMetrics + "\n";
		log += "  DisableEffectAreaMetrics: " + disableEffectAreaMetrics + "\n";
		log += "  EnableLocalEffectAreaMetrics: " + enableLocalEffectAreaMetrics + "\n";
		log += "  DisableCoordinatesMetrics: " + disableCoordinatesMetrics + "\n";
		log += "  DisableGeoCoordinatesFormat: " + disableGeoCoordinatesFormat + "\n";
		log += "  DisableRPCMetrics: " + disableRPCMetrics + "\n";
		log += "  DisableEventMetrics: " + disableEventMetrics + "\n";
		log += "  MapEffectiveSize: " + MetricZ_Geo.GetMapEffectiveSize() + "\n";
		log += "  MaxPlayers: " + maxPlayers + "\n";
		log += "  LimitFPS: " + limitFPS + "\n";
		log += "  BaseLabels: " + MetricZ_LabelUtils.BaseLabels();

		ErrorEx(log, ErrorExSeverity.INFO);
	}
}
#endif
