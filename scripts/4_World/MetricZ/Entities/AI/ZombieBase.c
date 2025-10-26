/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup and kills.
*/
modded class ZombieBase
{
	protected int m_MetricZ_State = -1; // last reported mind state

	/**
	    \brief Increment infected gauge on entity init.
	*/
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Infected.Inc();
	}

	/**
	    \brief Decrement infected gauge and update mind-state stats on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Infected.Dec();
		MetricZ_ZombieMindStats.OnDelete(m_MetricZ_State);

		super.EEDelete(parent);
	}

	/**
	    \brief Increment infected deaths counter on kill and reset cached state.
	*/
	override void EEKilled(Object killer)
	{
		MetricZ_Storage.s_InfectedDeaths.Inc();
		m_MetricZ_State = -1;

		super.EEKilled(killer);
	}

	/**
	    \brief Track mind state transitions for infected.
	    \details When state changes, decrement old and increment new in MetricZ_ZombieMindStats.
	    \return \p bool Base return value.
	*/
	bool HandleMindStateChange(int pCurrentCommandID, DayZInfectedInputController pInputController, float pDt)
	{
		bool ret = super.HandleMindStateChange(pCurrentCommandID, pInputController, pDt);

		if (m_MindState != m_MetricZ_State) {
			MetricZ_ZombieMindStats.OnStateChange(m_MetricZ_State, m_MindState);
			m_MetricZ_State = m_MindState;
		}

		return ret;
	}
}
#endif
