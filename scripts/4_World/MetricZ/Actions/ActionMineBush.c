/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
modded class ActionMineBush
{
	override void OnExecuteServer(ActionData action_data)
	{
		MetricZ_Storage.s_MinedBushes.Inc();

		super.OnExecuteServer(action_data);
	}
}

modded class ActionMineBushByHand
{
	override void OnExecuteServer(ActionData action_data)
	{
		MetricZ_Storage.s_MinedBushes.Inc();

		super.OnExecuteServer(action_data);
	}
}
#endif
