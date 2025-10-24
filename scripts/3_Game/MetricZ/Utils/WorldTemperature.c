/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief World temperature sampler without any object lookups.
    \details
    T(center) = BaseExact(sea) + AltDelta(center,t) + Overcast(base_center) + Wind(base_center) + Fog(base_center).
    AltDelta(center,t) = GetBaseEnvTemperatureAtPosition(center) - GetBaseEnvTemperature().
    BaseExact(sea) = GetBaseEnvTemperatureExact(month, day, hour, minute).
    Weather components are computed at base_center.
*/
class MetricZ_WorldTemperature
{
	protected static bool s_Ready; //!< one-time init flag
	protected static vector s_CenterSurf; //!< cached world center surface (terrain/water)

	/**
	    \brief Cache center surface once.
	*/
	static void Init()
	{
		if (s_Ready)
			return;

		float half = GetGame().GetWorld().GetWorldSize() * 0.5;
		float y = GetGame().SurfaceY(half, half);
		float depth = GetGame().GetWaterDepth(Vector(half, y, half));
		if (depth > 0)
			y += depth;

		s_CenterSurf = Vector(half, y, half);
		s_Ready = true;
	}

	/**
	    \brief Temperature at the map center including weather components. No object lookup required.
	    \return \p float Celsius
	*/
	static float Get()
	{
		Init();

		WorldData wd = g_Game.GetMission().GetWorldData();
		if (!wd)
			return 0.0;

		int y, m, d, hh, mm;
		GetGame().GetWorld().GetDate(y, m, d, hh, mm);

		// base sea-level for current world date/time
		float base_sea_exact = wd.GetBaseEnvTemperatureExact(m, d, hh, mm);

		// dynamic altitude correction at center (depends on current base env temp)
		float alt_delta = wd.GetBaseEnvTemperatureAtPosition(s_CenterSurf) - wd.GetBaseEnvTemperature();

		// base at center altitude
		float base_center = base_sea_exact + alt_delta;

		// add weather components computed from base_center
		float t_overcast = wd.GetTemperatureComponentValue(base_center, EEnvironmentTemperatureComponent.OVERCAST);
		float t_wind = wd.GetTemperatureComponentValue(base_center, EEnvironmentTemperatureComponent.WIND);
		float t_fog = wd.GetTemperatureComponentValue(base_center, EEnvironmentTemperatureComponent.FOG);

		return base_center + t_overcast + t_wind + t_fog;
	}
}
#endif
