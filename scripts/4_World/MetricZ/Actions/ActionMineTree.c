/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
modded class ActionMineTree
{
	override void OnExecuteServer(ActionData action_data)
	{
		MetricZ_Storage.s_MinedTrees.Inc();

		super.OnExecuteServer(action_data);
	}
}
#endif
