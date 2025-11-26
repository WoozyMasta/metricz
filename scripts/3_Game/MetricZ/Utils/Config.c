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
	static const string METRICS_FILE = "$profile:metricz.prom";
	static const string METRICS_TEMP = "$profile:metricz.tmp";
	static const string METRICS_CACHE = "$profile:metricz.cache";
	static const string NS = "dayz_metricz_";
	static const string CLI_FLAG_PREFIX = "metricz-";
	static const string CFG_OPT_PREFIX = "MetricZ_";

	// runtime overrides
	static int s_InitDelayMs;
	static int s_ScrapeIntervalMs;
	static bool s_DisablePlayerMetrics;
	static bool s_DisableZombieMetrics;
	static bool s_DisableAnimalMetrics;
	static bool s_DisableTransportMetrics;
	static bool s_DisableWeaponMetrics;
	static bool s_DisableEntityHitsMetrics;
	static float s_EntityHitDamageThreshold;
	static float s_EntityVehicleHitDamageThreshold;
	static bool s_DisableEntityKillsMetrics;
	static bool s_DisableTerritoryMetrics;
	static bool s_EnableCoordinatesMetrics;
	static bool s_DisableRPCMetrics;
	static bool s_DisableEventMetrics;

	// server params -> metrics
	static int s_MaxPlayers = 255; // from serverDZ.cfg:maxPlayers
	static int s_LimitFPS = 0; // from CLI:-limitFPS

	/**
	    \brief Load and apply configuration overrides.
	    \details Reads options from serverDZ.cfg (keys prefixed with "MetricZ_")
	            and CLI flags (prefixed with "metricz-"). CLI overrides config.
	            Converts seconds to milliseconds and enforces minimums.
	*/
	static void Load()
	{
		// Delay before the first metric collection in seconds
		s_InitDelayMs = GetNumber("InitDelay", "init-delay", 60, 0, 300) * 1000;

		// Interval between metric updates in seconds
		s_ScrapeIntervalMs = GetNumber("ScrapeInterval", "scrape-interval", 15, 1, 300) * 1000;

		// Disable player-related metrics collection
		// `dayz_metricz_player_*`
		s_DisablePlayerMetrics = GetBool("DisablePlayerMetrics", "disable-player");

		// Disable zombie per-type and mind states metrics collection
		// `dayz_metricz_animals_by_type`
		s_DisableZombieMetrics = GetBool("DisableZombieMetrics", "disable-zombie");

		// Disable animal per-type metrics collection
		// `dayz_metricz_infected_by_type` and `dayz_metricz_infected_mind_state`
		s_DisableAnimalMetrics = GetBool("DisableAnimalMetrics", "disable-animal");

		// Disable vehicle and transport metrics collection
		// `dayz_metricz_transport_*`
		s_DisableTransportMetrics = GetBool("DisableTransportMetrics", "disable-transport");

		// Disable weapon per-type (count, shoot, kills and hits) metrics collection
		// `dayz_metricz_weapon_shots_total`, `dayz_metricz_weapons_by_type_total`,
		// `dayz_metricz_player_killed_by_total` and `dayz_metricz_creature_killed_by_total`
		s_DisableWeaponMetrics = GetBool("DisableWeaponMetrics", "disable-weapon");

		// Disable player and zombie/animal hit by ammo type metrics
		// `dayz_metricz_player_killed_by_total` and `dayz_metricz_creature_killed_by_total`
		s_DisableEntityHitsMetrics = GetBool("DisableEntityHitsMetrics", "disable-entity-hits");

		// Minimum damage to log in `EEHitBy()`;
		// -1 and less disables threshold.
		s_EntityHitDamageThreshold = GetNumber("EntityHitDamageThreshold", "entity-hit-damage-threshold", 3, -1, 100);

		// Minimum vehicle-hit damage to log in `EEHitBy()`;
		// -1 and less disables threshold.
		s_EntityVehicleHitDamageThreshold = GetNumber("EntityVehicleHitDamageThreshold", "entity-vehicle-hit-damage-threshold", 15, -1, 100);

		// Disable territory flag metrics collection
		// `dayz_metricz_territory_lifetime`
		s_DisableTerritoryMetrics = GetBool("DisableTerritoryMetrics", "disable-territory");

		// Disable player and transport coordinate metrics
		s_DisableCoordinatesMetrics = GetBool("DisableCoordinatesMetrics", "disable-coordinates");

		// Disable conversion of coordinates metrics to geo `EPSG:4326` (WGS84) format.
		// By default convert position to lon/lat in `-180/180` and `-90/90` range.
		// If disable, all exported coordinates hold vanilla zero relative meters
		s_DisableGeoCoordinatesFormat = GetBool("DisableGeoCoordinatesFormat", "disable-geo-coordinates-format");

		// Disable RPC metrics collection
		// `dayz_metricz_rpc_input_total`
		s_DisableRPCMetrics = GetBool("DisableRPCMetrics", "disable-rpc");

		// Disable event handler metrics collection
		// `dayz_metricz_events_total`
		s_DisableEventMetrics = GetBool("DisableEventMetrics", "disable-event");

		// Override effective map tiles size in world units.
		// Useful if the web map size is larger than the game world size
		// (for example, the izurvive tiles for Chernarus have a size of `15926`, although the world size is `15360`)
		s_MapEffectiveSize = GetNumber("MapEffectiveSize", "map-effective-size", 0, g_Game.GetWorld().GetWorldSize());

		// Override map tiles name.
		// Useful if the name of the web map tiles was not recognized correctly
		s_MapTilesName = GetString("map-tiles-name");

		// Override map tiles version.
		// Useful if the web map version has been updated but the MetricZ returns the old version
		s_MapTilesVersion = GetString("map-tiles-version");

		// Override map tiles format (e.g. `webp`, `jpg`, `png`)
		s_MapTilesFormat = GetString("map-tiles-format");

		// server params
		int v = g_Game.ServerConfigGetInt("maxPlayers");
		if (v > 0)
			s_MaxPlayers = v;

		string flagValue;
		if (GetCLIParam("limitFPS", flagValue)) {
			int fps = flagValue.ToInt();
			if (fps > 0)
				s_LimitFPS = fps;
		}

		// Init geo cache
		MetricZ_Geo.Init();

		// Load labels cache
		MetricZ_PersistentCache.Load();

#ifdef DIAG
		DebugConfig();
#endif
	}

	/**
	    \brief Read float option from config and CLI.
	    \param cfgKey  Key suffix in serverDZ.cfg (without "MetricZ_").
	    \param cliFlag CLI flag suffix (without "metricz-").
	    \param defF    Default float value.
	    \param minF    Minimum allowed value.
	    \param maxF    Maximum allowed value.
	    \return Resolved float value.
	*/
	private static float GetNumber(string cfgKey, string cliFlag, float defF, float minF, float maxF)
	{
		float result = defF;

		float configVal = g_Game.ServerConfigGetInt(CFG_OPT_PREFIX + cfgKey);
		if (configVal != 0) {
			if (configVal < minF)
				configVal = minF;

			if (configVal >= minF)
				result = configVal;
		}

		string cliVal;
		if (GetCLIParam(CLI_FLAG_PREFIX + cliFlag, cliVal)) {
			float cliValFloat = cliVal.ToFloat();
			if (cliValFloat >= minF)
				result = cliValFloat;
		}

		if (result > maxF)
			return maxF;

		return result;
	}

	/**
	    \brief Read string from CLI.
	    \param cliFlag CLI flag suffix (without "metricz-").
	    \return Flag value or empty string.
	*/
	private static string GetString(string cliFlag)
	{
		string cliVal;
		if (GetCLIParam(CLI_FLAG_PREFIX + cliFlag, cliVal))
			return cliVal;

		return string.Empty;
	}

	/**
	    \brief Read a boolean toggle from config and CLI.
	    \details Config acts as enable-only (non-zero => true). CLI accepts
	            "true"/"false"/"1"/"0" and can disable. CLI takes precedence.
	    \param cfgKey  Key suffix in serverDZ.cfg (without "MetricZ_").
	    \param cliFlag CLI flag suffix (without "metricz-").
	    \return bool   Resolved toggle value.
	*/
	private static bool GetBool(string cfgKey, string cliFlag)
	{

		int configVal = g_Game.ServerConfigGetInt(CFG_OPT_PREFIX + cfgKey);
		bool result = (configVal != 0);

		string cliVal;
		if (GetCLIParam(CLI_FLAG_PREFIX + cliFlag, cliVal))
			result = (cliVal == "true" || cliVal == "1" || cliVal == string.Empty);

		return result;
	}

	private static void DebugConfig()
	{
		string log = "MetricZ configuration loaded:\n";

		log += "  InitDelayMs: " + s_InitDelayMs + "\n";
		log += "  ScrapeIntervalMs: " + s_ScrapeIntervalMs + "\n";
		log += "  DisablePlayerMetrics: " + s_DisablePlayerMetrics + "\n";
		log += "  DisableZombieMetrics: " + s_DisableZombieMetrics + "\n";
		log += "  DisableAnimalMetrics: " + s_DisableAnimalMetrics + "\n";
		log += "  DisableTransportMetrics: " + s_DisableTransportMetrics + "\n";
		log += "  DisableWeaponMetrics: " + s_DisableWeaponMetrics + "\n";
		log += "  DisableEntityHitsMetrics: " + s_DisableEntityHitsMetrics + "\n";
		log += "  EntityHitDamageThreshold: " + s_EntityHitDamageThreshold + "\n";
		log += "  EntityVehicleHitDamageThreshold: " + s_EntityVehicleHitDamageThreshold + "\n";
		log += "  DisableEntityKillsMetrics: " + s_DisableEntityKillsMetrics + "\n";
		log += "  DisableTerritoryMetrics: " + s_DisableTerritoryMetrics + "\n";
		log += "  EnableCoordinatesMetrics: " + s_EnableCoordinatesMetrics + "\n";
		log += "  DisableRPCMetrics: " + s_DisableRPCMetrics + "\n";
		log += "  DisableEventMetrics: " + s_DisableEventMetrics + "\n";
		log += "  MaxPlayers: " + s_MaxPlayers + "\n";
		log += "  LimitFPS: " + s_LimitFPS;

		ErrorEx(log, ErrorExSeverity.INFO);
	}
}
#endif
