/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
modded class ActionUnpackGift
{
	/**
	    \brief Count opened gift on action completion.
	*/
	override void OnFinishProgress(ActionData action_data)
	{
		MetricZ_Storage.s_GiftsUnpacked.Inc();

		super.OnFinishProgress(action_data);
	}
}
#endif
