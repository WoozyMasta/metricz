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
		// Delay before the first metric collection in seconds (default 60)
		s_InitDelayMs = Seconds("InitDelay", "init-delay", INIT_DELAY, 0);

		// Interval between metric updates in seconds (default 15, min 1)
		s_ScrapeIntervalMs = Seconds("ScrapeInterval", "scrape-interval", SCRAPE_INTERVAL, 1000);

		// Disable player-related metrics collection
		s_DisablePlayerMetrics = Toggle("DisablePlayerMetrics", "disable-player");

		// Disable zombie per-type and mind states metrics collection
		s_DisableZombieMetrics = Toggle("DisableZombieMetrics", "disable-zombie");

		// Disable animal per-type metrics collection
		s_DisableAnimalMetrics = Toggle("DisableAnimalMetrics", "disable-animal");

		// Disable vehicle and transport metrics collection
		s_DisableTransportMetrics = Toggle("DisableTransportMetrics", "disable-transport");

		// Disable weapon usage metrics collection
		s_DisableWeaponMetrics = Toggle("DisableWeaponMetrics", "disable-weapon");

		// Disable territory flag metrics collection
		s_DisableTerritoryMetrics = Toggle("DisableTerritoryMetrics", "disable-territory");

		// Enable player coordinate metrics (off by default)
		s_EnableCoordinatesMetrics = Toggle("EnableCoordinatesMetrics", "enable-coordinates");

		// Disable RPC metrics collection
		s_DisableRPCMetrics = Toggle("DisableRPCMetrics", "disable-rpc");

		// Disable event handler metrics collection
		s_DisableEventMetrics = Toggle("DisableEventMetrics", "disable-event");

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

		int configVal = g_Game.ServerConfigGetInt(CFG_OPT_PREFIX + cfgKey);
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
	    \return bool   Resolved toggle value.
	*/
	private static bool Toggle(string cfgKey, string cliFlag)
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
