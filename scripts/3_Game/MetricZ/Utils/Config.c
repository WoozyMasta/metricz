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
	static const string FILE = "$profile:metricz.prom";
	static const string TEMP = "$profile:metricz.tmp";
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
	static bool s_DisableTerritoryMetrics;
	static bool s_DisableCoordinatesMetrics;
	static bool s_DisableGeoCoordinatesFormat;
	static bool s_DisableRPCMetrics;
	static bool s_DisableEventMetrics;
	static float s_MapEffectiveSize;
	static string s_MapTilesName;
	static string s_MapTilesVersion;
	static string s_MapTilesFormat;

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
		s_InitDelayMs = GetNumber("InitDelay", "init-delay", 60, 0) * 1000;

		// Interval between metric updates in seconds
		s_ScrapeIntervalMs = GetNumber("ScrapeInterval", "scrape-interval", 15, 1) * 1000;

		// Disable player-related metrics collection
		s_DisablePlayerMetrics = GetBool("DisablePlayerMetrics", "disable-player");

		// Disable zombie per-type and mind states metrics collection
		s_DisableZombieMetrics = GetBool("DisableZombieMetrics", "disable-zombie");

		// Disable animal per-type metrics collection
		s_DisableAnimalMetrics = GetBool("DisableAnimalMetrics", "disable-animal");

		// Disable vehicle and transport metrics collection
		s_DisableTransportMetrics = GetBool("DisableTransportMetrics", "disable-transport");

		// Disable weapon usage metrics collection
		s_DisableWeaponMetrics = GetBool("DisableWeaponMetrics", "disable-weapon");

		// Disable territory flag metrics collection
		s_DisableTerritoryMetrics = GetBool("DisableTerritoryMetrics", "disable-territory");

		// Disable player and transport coordinate metrics
		s_DisableCoordinatesMetrics = GetBool("DisableCoordinatesMetrics", "disable-coordinates");

		// Disable conversion of coordinates metrics to geo `EPSG:4326` (WGS84) format.
		// By default convert position to lon/lat in `-180/180` and `-90/90` range.
		// If disable, all exported coordinates hold vanilla zero relative meters
		s_DisableGeoCoordinatesFormat = GetBool("DisableGeoCoordinatesFormat", "disable-geo-coordinates-format");

		// Disable RPC metrics collection
		s_DisableRPCMetrics = GetBool("DisableRPCMetrics", "disable-rpc");

		// Disable event handler metrics collection
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

		MetricZ_Geo.Init();
	}

	/**
	    \brief Read float option from config and CLI.
	    \param cfgKey  Key suffix in serverDZ.cfg (without "MetricZ_").
	    \param cliFlag CLI flag suffix (without "metricz-").
	    \param defF    Default float value.
	    \param minF    Minimum allowed value.
	    \return Resolved float value.
	*/
	private static float GetNumber(string cfgKey, string cliFlag, float defF, float minF = 0)
	{
		float result = defF;

		float configVal = g_Game.ServerConfigGetInt(CFG_OPT_PREFIX + cfgKey);
		if (configVal >= minF)
			result = configVal;

		string cliVal;
		if (GetCLIParam(CLI_FLAG_PREFIX + cliFlag, cliVal)) {
			float cliValFloat = cliVal.ToFloat();
			if (cliValFloat >= minF)
				result = cliValFloat;
		}

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
}
#endif
