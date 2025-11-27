/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Updates MetricZ artillery strike counter.
*/
modded class ContaminatedArea_Dynamic
{
	override void EEOnCECreate()
	{
		super.EEOnCECreate();

		if (m_DecayState == eAreaDecayStage.INIT)
			MetricZ_Storage.s_Artillery.Inc();
	}
}
#endif
