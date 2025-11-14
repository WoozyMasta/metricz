/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup and kills.
*/
modded class AnimalBase
{
	// cache for animal names used in labels: original -> canonical
	protected static ref map<string, string> s_MetricZ_LabelNames = new map<string, string>();

	/**
	    \brief Increment animal gauge on entity init.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (!MetricZ_Config.s_DisableAnimalMetrics)
			MetricZ_AnimalStats.OnSpawn(this);

		MetricZ_Storage.s_Animals.Inc();
	}

	/**
	    \brief Decrement animal gauge on entity delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		MetricZ_Storage.s_Animals.Dec();

		if (IsDamageDestroyed())
			MetricZ_Storage.s_AnimalsCorpses.Dec();

		if (!MetricZ_Config.s_DisableAnimalMetrics)
			MetricZ_AnimalStats.OnDelete(this);

		super.EEDelete(parent);
	}

	/**
	    \brief Increment animal deaths counter on kill.
	*/
	override void EEKilled(Object killer)
	{
		MetricZ_Storage.s_AnimalsDeaths.Inc();
		MetricZ_Storage.s_AnimalsCorpses.Inc();

		super.EEKilled(killer);
	}

	/**
	    \brief Public helper for MetricZ: returns cached canonical label name for this animal.
	    \details You can override this for set some beauty label name for your animal.
	*/
	string MetricZ_GetAnimalName()
	{
		return MetricZ_GetAnimalNameFromConfig();
	}

	/**
	    \brief Resolve canonical animal name using config (Steaks -> Pelt -> type) and cache it.
	    \details
	      - Try Skinning.ObtainedSteaks.item and strip:
	          "SteakMeat", "BreastMeat", "LegMeat"
	      - If empty, try Skinning.ObtainedPelt.item and strip "Pelt"
	      - If still empty, use GetType()
	      - Result is lowercased and cached per animal type
	*/
	protected string MetricZ_GetAnimalNameFromConfig()
	{
		string type = GetType();

		// cache hit
		string cached;
		if (s_MetricZ_LabelNames && s_MetricZ_LabelNames.Find(type, cached))
			return cached;

		string cfgBase = CFG_VEHICLESPATH + " " + type + " Skinning ";
		string result;

		// 1) Steaks
		if (GetGame().ConfigIsExisting(cfgBase + "ObtainedSteaks item")) {
			string steakItem;
			GetGame().ConfigGetText(cfgBase + "ObtainedSteaks item", steakItem);
			steakItem.TrimInPlace();

			if (steakItem != string.Empty) {
				// try supported meat suffixes
				string nameCandidate;

				nameCandidate = MetricZ_GetAnimalNameByItem(steakItem, "SteakMeat");
				if (nameCandidate == string.Empty)
					nameCandidate = MetricZ_GetAnimalNameByItem(steakItem, "BreastMeat");
				if (nameCandidate == string.Empty)
					nameCandidate = MetricZ_GetAnimalNameByItem(steakItem, "LegMeat");

				if (nameCandidate != string.Empty)
					result = nameCandidate;
			}
		}

		// 2) Pelt fallback
		if (result == string.Empty && GetGame().ConfigIsExisting(cfgBase + "ObtainedPelt item")) {
			string peltItem;
			GetGame().ConfigGetText(cfgBase + "ObtainedPelt item", peltItem);
			peltItem.TrimInPlace();

			if (peltItem != string.Empty) {
				string peltName = MetricZ_GetAnimalNameByItem(peltItem, "Pelt");
				if (peltName != string.Empty)
					result = peltName;
			}
		}

		// 3) Last fallback
		if (result == string.Empty)
			result = type;

		result.ToLower();

		// cache store
		if (!s_MetricZ_LabelNames)
			s_MetricZ_LabelNames = new map<string, string>();
		s_MetricZ_LabelNames.Set(type, result);

		return result;
	}

	/**
	    \brief Extract base animal name from item class name using suffix.
	    \param item Item classname from config (e.g. "CowSteakMeat")
	    \param suffix Expected suffix (e.g. "SteakMeat", "BreastMeat", "LegMeat", "Pelt")
	    \return Base name without suffix, or empty string on mismatch.
	*/
	protected string MetricZ_GetAnimalNameByItem(string item, string suffix)
	{
		item.TrimInPlace();
		if (item == string.Empty)
			return "";

		int lenItem = item.Length();
		int lenSuf = suffix.Length();

		if (lenSuf == 0 || lenSuf > lenItem)
			return "";

		int pos = item.LastIndexOf(suffix);
		if (pos == -1)
			return "";

		// suffix must be at the end
		if (pos + lenSuf != lenItem)
			return "";

		return item.Substring(0, pos);
	}
}
#endif
