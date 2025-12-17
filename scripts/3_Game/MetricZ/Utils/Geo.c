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
        - Can use own tiles server like https://github.com/WoozyMasta/dzmap
        - Work with stable tiles URL like https://dzmap.woozymasta.ru/maps/chernarusplus/topographic/3/6/1.webp
*/
class MetricZ_Geo
{
	static const float MAX_LAT = 85.05112878; //!< Web Mercator latitude clamp

	// world / projection parameters
	protected static float s_mapEffectiveSize; //!< Effective map size in world units
	protected static float s_LongitudeScale; //!< Precomputed scale for longitude (360 / size)
	protected static float s_MercatorScale; //!< Precomputed scale for Mercator Y ((2*PI) / size)

	/**
	    \brief Return effective map size in world units.
	*/
	static float GetMapEffectiveSize()
	{
		return s_mapEffectiveSize;
	}

	/**
	    \brief Convert object world position to EPSG:4326 (WGS84).
	    \param obj World object to sample position from.
	    \return Vector(lon, y, lat) or raw world vector.
	*/
	static vector GetPosition(Object obj)
	{
		if (!obj || !MetricZ_Config.IsLoaded())
			return vector.Zero;

		if (s_mapEffectiveSize <= 0)
			Init();

		vector pos = obj.GetPosition();
		if (MetricZ_Config.Get().geo.disable_transform_coordinates)
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
		if (!MetricZ_Config.IsLoaded())
			return;

		if (s_mapEffectiveSize <= 0)
			Init();

		if (MetricZ_Config.Get().geo.disable_transform_coordinates) {
			lon = pos[2];
			lat = pos[0];
			return;
		}

		// x: [0..size] -> lon: [-180..180]
		lon = pos[0] * s_LongitudeScale - 180.0;

		// z: [0..size] -> mercatorY: [-PI..PI]
		float mercatorY = pos[2] * s_MercatorScale - Math.PI;
		float latRad = (2.0 * Math.Atan(Math.Pow(Math.EULER, mercatorY))) - (Math.PI * 0.5);
		lat = Math.Clamp(latRad * Math.RAD2DEG, -MAX_LAT, MAX_LAT);
	}

	/**
	        \brief Convert radius in meters to degrees (relative to map projection).
	        \param radiusMeters Radius in game world meters.
	        \return Radius value in degrees (scaled to fit the projected world).
	*/
	static float GetRadiusDegrees(float radiusMeters)
	{
		if (!MetricZ_Config.IsLoaded())
			return 0;

		if (s_mapEffectiveSize <= 0)
			Init();

		if (MetricZ_Config.Get().geo.disable_transform_coordinates)
			return radiusMeters;

		return radiusMeters * s_LongitudeScale;
	}

	/**
	    \brief Initialize map size data
	*/
	static void Init()
	{
		if (!MetricZ_Config.IsLoaded())
			return;

		// base world size from engine
		s_mapEffectiveSize = g_Game.GetWorld().GetWorldSize();

		// apply config override
		if (MetricZ_Config.Get().geo.world_effective_size_resolved > 0)
			s_mapEffectiveSize = MetricZ_Config.Get().geo.world_effective_size_resolved;
		if (s_mapEffectiveSize <= 0)
			s_mapEffectiveSize = 15360;

		// recompute scales after override
		s_LongitudeScale = 360.0 / s_mapEffectiveSize;
		s_MercatorScale = (2.0 * Math.PI) / s_mapEffectiveSize;
	}
}
#endif
