/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
modded class ActionMineBush
{
	/**
	    \brief Count mined bush on action completion.
	*/
	override void OnFinishProgress(ActionData action_data)
	{
		MetricZ_Storage.s_MinedBushes.Inc();

		super.OnFinishProgress(action_data);
	}
}
#endif
