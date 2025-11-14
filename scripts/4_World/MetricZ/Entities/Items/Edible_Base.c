/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/
#ifdef SERVER
/**
    \brief Updates MetricZ counters on spawn/cleanup.
*/
modded class Edible_Base
{
	/**
	    \brief Increment food gauge on init.
	*/
	override void EEInit()
	{
		super.EEInit();

		if (!IsInherited(Bottle_Base))
			MetricZ_Storage.s_Food.Inc();

		MetricZ_Storage.FoodMetricChange(MetricZ_GetFoodType(), true);
	}

	/**
	    \brief Decrement food gauge on delete.
	*/
	override void EEDelete(EntityAI parent)
	{
		if (!IsInherited(Bottle_Base))
			MetricZ_Storage.s_Food.Dec();

		MetricZ_Storage.FoodMetricChange(MetricZ_GetFoodType(), false);

		super.EEDelete(parent);
	}

	MetricZ_FoodTypes MetricZ_GetFoodType()
	{
		// skip bottle base by exists "bottles" metric
		if (IsInherited(Bottle_Base))
			return MetricZ_FoodTypes.NONE;

		if (IsFruit())
			return MetricZ_FoodTypes.FRUIT;

		if (IsMushroom())
			return MetricZ_FoodTypes.MUSHROOM;

		if (IsCorpse())
			return MetricZ_FoodTypes.CORPSE;

		if (IsInherited(Worm))
			return MetricZ_FoodTypes.WORM;

		if (IsInherited(Guts) || IsInherited(SmallGuts))
			return MetricZ_FoodTypes.GUTS;

		if (IsMeat())
			return MetricZ_FoodTypes.MEAT;

		if (IsInherited(HumanSteakMeat))
			return MetricZ_FoodTypes.HUMAN_MEAT;

		if (GetLiquidType() == LIQUID_DISINFECTANT || GetLiquidTypeInit() == LIQUID_DISINFECTANT)
			return MetricZ_FoodTypes.DISINFECTANT;

		if (ConfigGetString("stackedUnit") == "pills")
			return MetricZ_FoodTypes.PILLS;

		if (IsInherited(SodaCan_ColorBase))
			return MetricZ_FoodTypes.DRINK;

		if (IsInherited(Snack_ColorBase) || IsInherited(Zagorky_ColorBase))
			return MetricZ_FoodTypes.SNACK;

		if (IsInherited(Candycane_Colorbase))
			return MetricZ_FoodTypes.CANDY;

		if (IsKindOf("FoodCan_100g_ColorBase"))
			return MetricZ_FoodTypes.CANNED_SMALL;

		if (IsKindOf("FoodCan_250g_ColorBase"))
			return MetricZ_FoodTypes.CANNED_MEDIUM;

		if (ConfigGetString("soundImpactType") == "metal")
			return MetricZ_FoodTypes.CANNED_BIG;

		if (ConfigGetString("soundImpactType") == "glass")
			return MetricZ_FoodTypes.JAR;

		return MetricZ_FoodTypes.OTHER;
	}
}

enum MetricZ_FoodTypes {
	NONE = -1,
	OTHER = 0,
	FRUIT,
	MUSHROOM,
	CORPSE,
	WORM,
	GUTS,
	MEAT,
	HUMAN_MEAT,
	DISINFECTANT,
	PILLS,
	DRINK,
	SNACK,
	CANDY,
	CANNED_SMALL,
	CANNED_MEDIUM,
	CANNED_BIG,
	JAR,
}
#endif
