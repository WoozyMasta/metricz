/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
modded class ActionUnpackGift
{
	override void OnFinishProgressServer(ActionData action_data)
	{
		MetricZ_Storage.s_GiftsUnpacked.Inc();

		super.OnFinishProgressServer(action_data);
	}
}
#endif
