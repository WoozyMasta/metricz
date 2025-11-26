/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Geo helpers for world position and map tiles metadata.
    \details
        - Converts DayZ world coordinates (x,y,z) to EPSG:4326 (WGS84) lon/lat.
        - Provides map tile metadata (name, version, format, workshop ID).
        - Uses MetricZ_Config overrides when present.
*/
class MetricZ_Geo
{
	static const float MAX_LAT = 85.05112878; //!< Web Mercator latitude clamp
	static const float FALLBACK_SIZE = 15360; //!< Fallback size (chernarusplus)

	// world / projection parameters
	protected static float s_MapEffectiveSize; //!< Effective map size in world units
	protected static float s_LongitudeScale; //!< Precomputed scale for longitude (360 / size)
	protected static float s_MercatorScale; //!< Precomputed scale for Mercator Y ((2*PI) / size)

	// tiles metadata
	protected static string s_MapTilesName; //!< Normalized map name for tiles
	protected static string s_MapTilesVersion; //!< Tiles version/tag
	protected static string s_MapTilesFormat; //!< Tiles image format (webp/jpg/...)
	protected static int s_MapWorkShopID; //!< Workshop ID or app ID

	/**
	    \brief Return effective map size in world units.
	*/
	static float GetMapEffectiveSize()
	{
		return s_MapEffectiveSize;
	}

	/**
	    \brief Return normalized map tiles name.
	*/
	static string GetMapTilesName()
	{
		return s_MapTilesName;
	}

	/**
	    \brief Return map tiles version string.
	*/
	static string GetMapTilesVersion()
	{
		return s_MapTilesVersion;
	}

	/**
	    \brief Return map tiles image format.
	*/
	static string GetMapTilesFormat()
	{
		return s_MapTilesFormat;
	}

	/**
	    \brief Build Steam Workshop URL for current map.
	*/
	static string GetWorkshopUrl()
	{
		if (s_MapWorkShopID < 100000000)
			return "https://steamcommunity.com/sharedfiles/filedetails/?id=" + s_MapWorkShopID;

		return "https://steamcommunity.com/app/" + s_MapWorkShopID;
	}

	/**
	    \brief Return Workshop ID or app ID for current map.
	*/
	static int GetWorkshopID()
	{
		return s_MapWorkShopID;
	}

	/**
	    \brief Convert object world position to EPSG:4326 (WGS84).
	    \param obj World object to sample position from.
	    \return Vector(lon, y, lat) or raw world vector.
	*/
	static vector GetPosition(Object obj)
	{
		if (!obj)
			return vector.Zero;

		if (s_MapEffectiveSize <= 0)
			Init();

		vector pos = obj.GetPosition();
		if (MetricZ_Config.s_DisableGeoCoordinatesFormat)
			return pos;

		float lon, lat;
		GetLonLat(pos, lon, lat);
		return Vector(lon, pos[1], lat);
	}

	/**
	    \brief Convert world position to lon/lat in EPSG:4326 (WGS84).
	    \param pos World position vector (x,y,z).
	    \param[out] lon Longitude in degrees.
	    \param[out] lat Latitude in degrees, clamped.
	*/
	static void GetLonLat(vector pos, out float lon, out float lat)
	{
		if (s_MapEffectiveSize <= 0)
			Init();

		// x: [0..size] -> lon: [-180..180]
		lon = pos[0] * s_LongitudeScale - 180.0;

		// z: [0..size] -> mercatorY: [-π..π]
		float mercatorY = pos[2] * s_MercatorScale - Math.PI;
		float latRad = (2.0 * Math.Atan(Math.Pow(Math.EULER, mercatorY))) - (Math.PI * 0.5);
		lat = Math.Clamp(latRad * Math.RAD2DEG, -MAX_LAT, MAX_LAT);
	}

	/**
	    \brief Initialize geo projection and tiles metadata.
	    \details
	        - Reads world size and name from engine.
	        - Fills tiles metadata based on normalized map name.
	        - Applies MetricZ_Config overrides for size/name/version/format.
	        - Precomputes longitude and Mercator scales.
	*/
	static void Init()
	{
		// base world size from engine
		s_MapEffectiveSize = g_Game.GetWorld().GetWorldSize();

		// base map name from engine
		g_Game.GetWorldName(s_MapTilesName);
		s_MapTilesName.TrimInPlace();
		s_MapTilesName.ToLower();
		if (s_MapTilesName == string.Empty)
			s_MapTilesName == "empty";

		// Map tiles version 1763446098241.bc53b35d
		switch (s_MapTilesName) {
		case "alteria": // size 8192
			s_MapTilesVersion = "1.0";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3296994216;
			break;

		case "anastara": // size 10240
			s_MapTilesVersion = "Jan.15";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2973953648;
			break;

		case "antoria": // size 20480
			s_MapTilesVersion = "25.05.18";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2966495799;
			break;

		case "arsteinen": // size 15360
			s_MapTilesVersion = "May.28";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2982575649;
			break;

		case "avalon": // size 20480
			s_MapTilesVersion = "25.05.16";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2982575649;
			break;

		case "azalea": // size 20480
			s_MapTilesVersion = "25.09.24";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3225634444;
			break;

		case "banovfrost":
		case "banov": // size 15360
			s_MapTilesVersion = "24.12.15-1";
			s_MapTilesFormat = "webp";
			s_MapTilesName = "banov";
			s_MapWorkShopID = 2415195639;
			break;

		case "bitterroot": // size 12288
			s_MapTilesVersion = "Aug.1";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2906823750;
			break;

		case "chernarusplus": // size 15360
			s_MapTilesVersion = "1.27";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 221100;
			break;

		case "chiemsee": // size 10240
			s_MapTilesVersion = "2.7";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 1580589252;
			break;

		case "deadfall": // size 10240
			s_MapTilesVersion = "25.07.06";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3050117454;
			break;

		case "deerisle": // size 16384
			s_MapTilesVersion = "5.923";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 1602372402;
			break;

		case "esseker": // size 12800
			s_MapTilesVersion = "0.58";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 2462896799;
			break;

		case "hashima": // size 5120
			s_MapTilesVersion = "25.03.30";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2781560371;
			break;

		case "iztek": // size 8192
			s_MapTilesVersion = "2.0.4";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 2978912938;
			break;

		case "enoch":
		case "livonia": // size 12800
			s_MapTilesVersion = "1.27";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 221100;
			s_MapTilesName = "livonia";
			break;

		case "malvinas": // size 12288
			s_MapTilesVersion = "25.09.17";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3390173486;
			break;

		case "melkart": // size 20480
			s_MapTilesVersion = "1.04";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2716445223;
			break;

		case "namalsk": // size 12800
			s_MapTilesVersion = "May.27";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2289456201;
			break;

		case "nhchernobyl": // size 20480
			s_MapTilesVersion = "Mar.13";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2727569951;
			break;

		case "novikostok": // size 15360
			s_MapTilesVersion = "25.11.11";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3553075906;
			break;

		case "nukezzone": // size 15360
			s_MapTilesVersion = "Mar.13";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2828800987;
			break;

		case "nyheim": // size 15360
			s_MapTilesVersion = "25.09.22";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2633522605;
			break;

		case "onforin": // size 24576
			s_MapTilesVersion = "Apr.8";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3445058656;
			break;

		case "pnw": // size 10240
			s_MapTilesVersion = "25.08.28";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3290318225;
			break;

		case "pripyat": // size 20480
			s_MapTilesVersion = "19.08";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 3136720512;
			break;

		case "raman": // size 32768
			s_MapTilesVersion = "Mar.14";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3401182744;
			break;

		case "ros": // size 20480
			s_MapTilesVersion = "Jul.28";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2390417799;
			break;

		case "rostow": // size 14336
			s_MapTilesVersion = "02.15";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 2344585107;
			break;

		case "sahrani": // size 20480
			s_MapTilesVersion = "25.06.25";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3508468461;
			break;

		case "sakhal": // size 15360
			s_MapTilesVersion = "1.27";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2968040;
			break;

		case "stuartisland": // size 5120
			s_MapTilesVersion = "20.04.27";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 1936423383;
			break;

		case "swansisland": // size 2048
			s_MapTilesVersion = "23.09.05";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2517396668;
			break;

		case "takistanplus": // size 12800
			s_MapTilesVersion = "1.041";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 2563233742;
			break;

		case "valning": // size 10240
			s_MapTilesVersion = "26.11";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 1880753439;
			break;

		case "vela": // size 10240
			s_MapTilesVersion = "0.5";
			s_MapTilesFormat = "jpg";
			s_MapWorkShopID = 2794308565;
			break;

		case "visisland": // size 20480
			s_MapTilesVersion = "0.04";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 3031438368;
			break;

		case "yiprit": // size 20480
			s_MapTilesVersion = "31.07";
			s_MapTilesFormat = "webp";
			s_MapWorkShopID = 2780320171;
			break;

		default:
			string game_version;
			g_Game.GetVersion(game_version);
			game_version.TrimInPlace();
			game_version.ToLower();

			array<string> parts = new array<string>();
			game_version.Split(".", parts);

			if (parts.Count() > 1)
				s_MapTilesVersion = parts[0] + "." + parts[1];
			else
				s_MapTilesVersion = game_version;

			s_MapTilesFormat = "webp";
			break;
		}

		// apply config overrides
		if (MetricZ_Config.s_MapEffectiveSize > 0)
			s_MapEffectiveSize = MetricZ_Config.s_MapEffectiveSize;
		if (s_MapEffectiveSize <= 0)
			s_MapEffectiveSize = FALLBACK_SIZE; // safety guard

		if (MetricZ_Config.s_MapTilesName != string.Empty)
			s_MapTilesName = MetricZ_Config.s_MapTilesName;

		if (MetricZ_Config.s_MapTilesVersion != string.Empty)
			s_MapTilesVersion = MetricZ_Config.s_MapTilesVersion;

		if (MetricZ_Config.s_MapTilesFormat != string.Empty)
			s_MapTilesFormat = MetricZ_Config.s_MapTilesFormat;

		// recompute scales after all overrides
		s_LongitudeScale = 360.0 / s_MapEffectiveSize;
		s_MercatorScale = (2.0 * Math.PI) / s_MapEffectiveSize;
	}
}
#endif
