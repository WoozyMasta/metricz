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
	// last reported mind state
	protected int m_MetricZ_State = -1;
	// cache for zombie types used in labels: original class -> canonical type
	protected static ref map<string, string> s_MetricZ_LabelNames = new map<string, string>();

	/**
	    \brief Increment infected gauge on entity init.
	*/
	override void EEInit()
	{
		super.EEInit();

		MetricZ_Storage.s_Infected.Inc();

		if (!MetricZ_Config.s_DisableZombieMetrics)
			MetricZ_ZombieStats.OnSpawn(this);
	}

	/**
	    \brief Decrement infected gauge and update mind-state stats on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Infected.Dec();

		if (IsDamageDestroyed())
			MetricZ_Storage.s_InfectedCorpses.Dec();

		if (!MetricZ_Config.s_DisableZombieMetrics)
			MetricZ_ZombieStats.OnDelete(this, m_MetricZ_State);

		super.EEDelete(parent);
	}

	/**
	    \brief Increment infected deaths counter on kill and reset cached state.
	*/
	override void EEKilled(Object killer)
	{
		MetricZ_Storage.s_InfectedDeaths.Inc();
		MetricZ_Storage.s_InfectedCorpses.Inc();

		if (!MetricZ_Config.s_DisableZombieMetrics) {
			MetricZ_ZombieStats.OnKilled(m_MetricZ_State);
			m_MetricZ_State = MetricZ_ZombieStats.MINDSTATE_DEAD;
		} else
			m_MetricZ_State = -1;

		super.EEKilled(killer);
	}

	/**
	    \brief Track mind state transitions for infected.
	    \details When state changes, decrement old and increment new in MetricZ_ZombieStats.
	    \return \p bool Base return value.
	*/
	override bool HandleMindStateChange(int pCurrentCommandID, DayZInfectedInputController pInputController, float pDt)
	{
		bool ret = super.HandleMindStateChange(pCurrentCommandID, pInputController, pDt);

		if (m_MindState != m_MetricZ_State) {
			MetricZ_ZombieStats.OnStateChange(m_MetricZ_State, m_MindState);
			m_MetricZ_State = m_MindState;
		}

		return ret;
	}

	/**
	    \brief Public helper for MetricZ: returns cached canonical zombie type.
	    \details
	      You can override this for set some beauty label name for your zombie.
	      Must be lower_underscored string.
	*/
	string MetricZ_GetZombieType()
	{
		return MetricZ_GetZombieTypeFromConfig();
	}

	/**
	    \brief Resolve canonical zombie type using ResistContaminatedEffect and aiAgentTemplate.
	    \details
	      1) If ResistContaminatedEffect() -> "contaminated"
	      2) Read CFG_VEHICLESPATH <class> aiAgentTemplate
	         - "Infected" / "InfectedFemale" / "InfectedMale":
	             - if IsZombieMilitary() -> "military"
	             - else -> "default" / "default_female" / "default_male"
	         - any other template:
	             - strip leading "Infected" if present
	             - trim, lowercase
	      3) If empty or missing template:
	         - if IsZombieMilitary() -> "military"
	         - else -> class name (lowercased)
	      Result is cached per class name.
	*/
	protected string MetricZ_GetZombieTypeFromConfig()
	{
		// contamination has highest priority
		if (ResistContaminatedEffect())
			return "contaminated";

		string type = GetType();

		// cache hit
		string cached;
		if (s_MetricZ_LabelNames && s_MetricZ_LabelNames.Find(type, cached))
			return cached;

		string result;

		// read aiAgentTemplate from config
		string cfgPath = CFG_VEHICLESPATH + " " + type + " aiAgentTemplate";
		string tpl;

		if (GetGame().ConfigIsExisting(cfgPath)) {
			GetGame().ConfigGetText(cfgPath, tpl);
			tpl.TrimInPlace();

			if (tpl != string.Empty) {
				if (tpl == "Infected" || tpl == "InfectedFemale" || tpl == "InfectedMale") {
					if (IsZombieMilitary())
						result = "military";
					else {
						if (tpl == "InfectedFemale")
							result = "default_female";
						else if (tpl == "InfectedMale")
							result = "default_male";
						else
							result = "default";
					}
				} else {
					// non-base templates: strip "Infected" prefix if present
					string prefix = "Infected";
					int plen = prefix.Length();

					if (tpl.Length() >= plen && tpl.Substring(0, plen) == prefix)
						result = tpl.Substring(plen, tpl.Length() - plen);
					else {
						result = tpl; // custom template without prefix
					}
				}
			}
		}

		// fallback path if template missing/empty
		if (result == string.Empty) {
			if (IsZombieMilitary())
				result = "military";
			else
				result = type;
		}

		result.TrimInPlace();
		result.ToLower();

		if (result == string.Empty) {
			result = type;
			result.TrimInPlace();
			result.ToLower();
		}

		// cache store
		if (!s_MetricZ_LabelNames)
			s_MetricZ_LabelNames = new map<string, string>();
		s_MetricZ_LabelNames.Set(type, result);

		return result;
	}
}
#endif
