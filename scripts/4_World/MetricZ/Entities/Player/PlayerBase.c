/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Update MetricZ counters on spawn/cleanup and kills, and register a per-player metrics instance.
*/
modded class PlayerBase
{
	// prevent counting hits on death bodies except last hit
	protected bool m_MetricZ_IsLastHit;
	protected ref MetricZ_PlayerMetrics m_MetricZ;

	/**
	    \brief Register per-player metrics collector on init.
	    \details No-op if player metrics disabled.
	*/
	override void EEInit()
	{
		super.EEInit();

#ifdef EXPANSIONMODAI
		if (IsInherited(eAIBase)) {
			if (IsInherited(eAINPCBase))
				MetricZ_Storage.s_ExpansionAINPC.Inc();
			else
				MetricZ_Storage.s_ExpansionAI.Inc();

			return;
		}
#endif

		if (MetricZ_Config.s_DisablePlayerMetrics)
			return;

		if (!m_MetricZ) {
			m_MetricZ = new MetricZ_PlayerMetrics();
			m_MetricZ.Init(this);
		}
	}

	/**
	    \brief Drop metrics reference on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
#ifdef EXPANSIONMODAI
		if (IsInherited(eAIBase)) {
			if (IsInherited(eAINPCBase))
				MetricZ_Storage.s_ExpansionAINPC.Dec();
			else
				MetricZ_Storage.s_ExpansionAI.Dec();
		}
#endif

		m_MetricZ = null;

		super.EEDelete(parent);
	}

	/**
	    \brief Increment players_deaths counter on kill.
	*/
	override void EEKilled(Object killer)
	{
#ifdef EXPANSIONMODAI
		if (IsInherited(eAIBase)) {
			if (!MetricZ_Config.s_DisableEntityKillsMetrics && killer != this)
				MetricZ_WeaponStats.OnCreatureKill(killer);

			super.EEKilled(killer);

			return;
		}
#endif

		if (!MetricZ_Config.s_DisablePlayerMetrics)
			MetricZ_Storage.s_PlayersDeaths.Inc();

		if (!MetricZ_Config.s_DisableWeaponMetrics && killer != this)
			MetricZ_WeaponStats.OnPlayerKilled(killer);

		super.EEKilled(killer);
	}

	/**
	    \brief Capture hit statistics for metric counters.
	*/
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!MetricZ_Config.s_DisableEntityHitsMetrics && source != this && !m_MetricZ_IsLastHit) {
			if (IsDamageDestroyed())
				m_MetricZ_IsLastHit = true;

			if (damageResult) {
				float damage = damageResult.GetDamage(dmgZone, "");
				if (damage < MetricZ_Config.s_EntityHitDamageThreshold)
					return;

				if (source && source.IsTransport() && damage < MetricZ_Config.s_EntityVehicleHitDamageThreshold)
					return;
			}

#ifdef EXPANSIONMODAI
			if (IsInherited(eAIBase)) {
				MetricZ_HitStats.OnCreatureHit(ammo);
				return;
			}
#endif

			MetricZ_HitStats.OnPlayerHit(ammo);
		}
	}

	/**
	    \brief Accessor for per-player metrics.
	    \return \p MetricZ_PlayerMetrics or null if not initialized.
	*/
	MetricZ_PlayerMetrics MetricZ_GetMetrics()
	{
		return m_MetricZ;
	}

	/**
	    \brief Update player network stats.
	*/
	override void EOnPostFrame(IEntity other, int extra)
	{
		super.EOnPostFrame(other, extra);

		if (m_MetricZ || !MetricZ_Config.s_DisablePlayerMetrics)
			m_MetricZ.SampleNetwork();
	}
}
#endif
