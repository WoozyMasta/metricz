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
	protected int m_MetricZ_InsidersCount;
	protected ref MetricZ_EffectAreaMetrics m_MetricZ;

	/**
	    \brief Standard entity initialization.
	    \details
	        MetricZ initialization is skipped here. For dynamic zones, EEInit triggers
	        while the artillery projectile is still airborne. We must wait for the actual zone creation.
	*/
	override void EEInit()
	{
		super.EEInit();

		// MetricZ_Init();
	}

	/**
	    \brief Finalizes zone setup and registers metrics collector.
	    \details This is the safe point to initialize metrics:
	      - For Dynamic Zones: Called only after the shell has landed and the zone is physically created.
	      - For Static Zones: Called immediately.
	      Guarantees that metrics are not exported before the zone actually exists.
	*/
	override void InitZone()
	{
		super.InitZone();

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
	    \brief Get area radius.
	    \return \p int Radius.
	*/
	float MetricZ_GetRadius()
	{
		return m_Radius;
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
		if (!MetricZ_Config.IsLoaded() || m_MetricZ_AreaInit)
			return;

		if (MetricZ_Config.Get().disableLocalEffectAreaMetrics && IsInherited(ContaminatedArea_Local))
			return;

		m_MetricZ_AreaInit = true;
		MetricZ_Storage.s_EffectAreas.Inc();

		if (MetricZ_Config.Get().disableEffectAreaMetrics)
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
		if (!MetricZ_Config.IsLoaded() || !m_MetricZ_AreaInit)
			return;

		if (!MetricZ_Config.Get().disableEffectAreaMetrics) {
			m_MetricZ = null;
			MetricZ_EffectAreaRegistry.Unregister(this);
		}

		MetricZ_Storage.s_EffectAreas.Dec();
	}
}
#endif
