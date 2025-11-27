/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class EffectArea
{
	protected bool m_MetricZ_AreaInit;
	protected bool m_MetricZ_ZoneInit;
	protected int m_MetricZ_InsidersCount;
	protected ref MetricZ_EffectAreaMetrics m_MetricZ;

	/**
	    \brief Track EffectArea and register metrics collector.
	    \details Called in static and dynamic zones.
	*/
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Init();
	}

	/**
	    \brief Track EffectArea and register metrics collector.
	    \details Called for ContaminatedArea_Local or areas that do not call EEInit fully or need late setup.
	*/
	override void InitZone()
	{
		super.InitZone();

		m_MetricZ_ZoneInit = true;
		MetricZ_Init();
	}

	/**
	    \brief Unregister metrics and decrement gauge on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Delete();

		super.EEDelete(parent);
	}

	/**
	    \brief Increase insiders players gauge.
	*/
	override void OnPlayerEnterServer(PlayerBase player, EffectTrigger trigger)
	{
		super.OnPlayerEnterServer(player, trigger);

		if (player)
			m_MetricZ_InsidersCount++;
	}

	/**
	    \brief Decrease insiders players gauge.
	*/
	override void OnPlayerExitServer(PlayerBase player, EffectTrigger trigger)
	{
		super.OnPlayerExitServer(player, trigger);

		if (player)
			m_MetricZ_InsidersCount--;
	}

	/**
	    \brief Get string representation of zone type.
	    \return \p string Enum eZoneType as string.
	*/
	string MetricZ_GetType()
	{
		return EnumTools.EnumToString(eZoneType, m_Type);
	}

	/**
	    \brief Get area radius in degrees (MetricZ projection).
	    \return \p int Radius or 0 if not initialized.
	*/
	float MetricZ_GetRadius()
	{
		if (!m_MetricZ_AreaInit || !m_MetricZ_ZoneInit)
			return 0;

		return MetricZ_Geo.GetRadiusDegrees(m_Radius);
	}

	/**
	    \brief Get count of players currently inside the zone.
	    \return \p int
	*/
	int MetricZ_GetInsidersCount()
	{
		return m_MetricZ_InsidersCount;
	}

	/**
	    \brief Accessor for per-area metrics.
	    \return \p MetricZ_EffectAreaMetrics or null.
	*/
	MetricZ_EffectAreaMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}

	/**
	    \brief Initialize metrics tracking for this zone.
	    \details Updates global gauge, registers in MetricZ_EffectAreaRegistry, creates metric instance.
	*/
	protected void MetricZ_Init()
	{
		if (m_MetricZ_AreaInit)
			return;

		if (!MetricZ_Config.s_EnableLocalEffectAreaMetrics && IsInherited(ContaminatedArea_Local))
			return;

		m_MetricZ_AreaInit = true;
		MetricZ_Storage.s_EffectAreas.Inc();

		if (MetricZ_Config.s_DisableEffectAreaMetrics)
			return;

		MetricZ_EffectAreaRegistry.Register(this);

		if (!m_MetricZ) {
			m_MetricZ = new MetricZ_EffectAreaMetrics();
			m_MetricZ.Init(this);
		}
	}

	/**
	    \brief Cleanup metrics on deletion.
	    \details Updates global gauge, unregisters from registry, destroys metric instance.
	*/
	protected void MetricZ_Delete()
	{
		if (!m_MetricZ_AreaInit)
			return;

		if (!MetricZ_Config.s_DisableEffectAreaMetrics) {
			m_MetricZ = null;
			MetricZ_EffectAreaRegistry.Unregister(this);
		}

		MetricZ_Storage.s_EffectAreas.Dec();
	}
}
#endif
