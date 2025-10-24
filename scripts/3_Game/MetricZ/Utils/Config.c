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

	static const int INIT_DELAY = 60000;
	static const int SCRAPE_INTERVAL = 15000;

	// runtime overrides
	static int s_InitDelayMs = INIT_DELAY;
	static int s_ScrapeIntervalMs = SCRAPE_INTERVAL;
	static bool s_DisablePlayerMetrics = false;
	static bool s_DisableTransportMetrics = false;
	static bool s_DisableWeaponMetrics = false;
	static bool s_DisableTerritoryMetrics = false;
	static bool s_EnableCoordinatesMetrics = false;
	static bool s_DisableRPCMetrics = false;
	static bool s_DisableEventMetrics = false;

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
		// Delay before the first metric collection in seconds (default 60)
		s_InitDelayMs = Seconds("InitDelay", "init-delay", INIT_DELAY, 0);

		// Interval between metric updates in seconds (default 15, min 1)
		s_ScrapeIntervalMs = Seconds("ScrapeInterval", "scrape-interval", SCRAPE_INTERVAL, 1000);

		// Disable player-related metrics collection
		s_DisablePlayerMetrics = Toggle("DisablePlayerMetrics", "disable-player", s_DisablePlayerMetrics);

		// Disable vehicle and transport metrics collection
		s_DisableTransportMetrics = Toggle("DisableTransportMetrics", "disable-transport", s_DisableTransportMetrics);

		// Disable weapon usage metrics collection
		s_DisableWeaponMetrics = Toggle("DisableWeaponMetrics", "disable-weapon", s_DisableWeaponMetrics);

		// Disable territory flag metrics collection
		s_DisableTerritoryMetrics = Toggle("DisableTerritoryMetrics", "disable-territory", s_DisableTerritoryMetrics);

		// Enable player coordinate metrics (off by default)
		s_EnableCoordinatesMetrics = Toggle("EnableCoordinatesMetrics", "enable-coordinates", s_EnableCoordinatesMetrics);

		// Disable RPC metrics collection
		s_DisableRPCMetrics = Toggle("DisableRPCMetrics", "disable-rpc", s_DisableRPCMetrics);

		// Disable event handler metrics collection
		s_DisableEventMetrics = Toggle("DisableEventMetrics", "disable-event", s_DisableEventMetrics);

		// server params
		int v = GetGame().ServerConfigGetInt("maxPlayers");
		if (v > 0)
			s_MaxPlayers = v;

		string flagValue;
		if (GetCLIParam("limitFPS", flagValue)) {
			int fps = flagValue.ToInt();
			if (fps > 0)
				s_LimitFPS = fps;
		}
	}

	/**
	    \brief Read a time option in seconds and return milliseconds.
	    \param cfgKey   Key suffix in serverDZ.cfg (without "MetricZ_").
	    \param cliFlag  CLI flag suffix (without "metricz-").
	    \param defMs    Default value in milliseconds.
	    \param minMs    Minimum allowed value in milliseconds.
	    \return int     Milliseconds after applying overrides and minimum.
	*/
	private static int Seconds(string cfgKey, string cliFlag, int defMs, int minMs = 0)
	{
		int ms = defMs;

		int configVal = GetGame().ServerConfigGetInt(CFG_OPT_PREFIX + cfgKey);
		if (configVal > 0)
			ms = configVal * 1000;

		string cliVal;
		if (GetCLIParam(CLI_FLAG_PREFIX + cliFlag, cliVal)) {
			int cliValSec = cliVal.ToInt();
			if (cliValSec > 0)
				ms = cliValSec * 1000;
		}

		if (ms < minMs)
			ms = minMs;

		return ms;
	}

	/**
	    \brief Read a boolean toggle from config and CLI.
	    \details Config acts as enable-only (non-zero => true). CLI accepts
	            "true"/"false"/"1"/"0" and can disable. CLI takes precedence.
	    \param cfgKey  Key suffix in serverDZ.cfg (without "MetricZ_").
	    \param cliFlag CLI flag suffix (without "metricz-").
	    \param def     Default value when neither source is provided.
	    \return bool   Resolved toggle value.
	*/
	private static bool Toggle(string cfgKey, string cliFlag, bool def)
	{
		bool result = def;
		int configVal = GetGame().ServerConfigGetInt(CFG_OPT_PREFIX + cfgKey);
		if (configVal != 0)
			result = true; // cfg only enables

		string cliVal;
		if (GetCLIParam(CLI_FLAG_PREFIX + cliFlag, cliVal))
			result = IsBool(cliVal, true); // CLI can enable/disable

		return result;
	}

	/**
	    \brief Parse a string to bool.
	    \details Case-insensitive. Trims whitespace. Accepts "1","true","0","false".
	            Empty string returns \p emptyIsTrue.
	    \param value         Input string (modified by ToLower/TrimInPlace).
	    \param emptyIsTrue   Return value when input is empty.
	    \return bool         Parsed result.
	*/
	private static bool IsBool(string value, bool emptyIsTrue = true)
	{
		value.ToLower();
		value.TrimInPlace();

		if (value == "1" || value == "true")
			return true;

		if (value == "0" || value == "false")
			return false;

		if (value == string.Empty && emptyIsTrue)
			return true;

		return false;
	}
}
#endif
