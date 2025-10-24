/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
modded class ActionFishingNew: ActionContinuousBase
{
	override void OnEndInput(ActionData action_data)
	{
		MetricZ_Storage.s_FishingAttempts.Inc();

		super.OnEndInput(action_data);
	}

	override protected EntityAI TrySpawnCatch(FishingActionData action_data)
	{
		EntityAI catch = super.TrySpawnCatch(action_data);
		if (catch)
			MetricZ_Storage.s_FishingCatches.Inc();

		return catch;
	}
}
#endif
