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

		// Enable player and transport coordinate metrics
		s_EnableCoordinatesMetrics = GetBool("EnableCoordinatesMetrics", "enable-coordinates");

		// Disable RPC metrics collection
		s_DisableRPCMetrics = GetBool("DisableRPCMetrics", "disable-rpc");

		// Disable event handler metrics collection
		s_DisableEventMetrics = GetBool("DisableEventMetrics", "disable-event");

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

		// Load labels cache
		MetricZ_PersistentCache.Load();
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
