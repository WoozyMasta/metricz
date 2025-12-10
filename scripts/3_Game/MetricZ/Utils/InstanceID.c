/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Helper for get instance ID.
*/
class MetricZ_InstanceID
{
	static string s_InstanceID;

	static string Get()
	{
		if (s_InstanceID != string.Empty)
			return s_InstanceID;

		string id;

		// get override from config
		if (MetricZ_Config.IsLoaded()) {
			id = MetricZ_Config.Get().instance_id;
			if (id != string.Empty) {
				s_InstanceID = id;
				return s_InstanceID;
			}
		}

		// get configured instance id in server.cfg
		id = g_Game.ServerConfigGetInt("instanceId").ToString();
		if (id != "0") {
			s_InstanceID = id;
			return s_InstanceID;
		}

		// use game port as instance id
		GetCLIParam("port", id);
		if (id != "0") {
			ErrorEx(
			    "MetricZ instanceId is 0. Used game port " + id + " as instanceId.",
			    ErrorExSeverity.INFO);

			s_InstanceID = id;
			return s_InstanceID;
		}

		// use steam query port as instance id
		id = g_Game.ServerConfigGetInt("steamQueryPort").ToString();
		if (id != "0") {
			ErrorEx(
			    "MetricZ instanceId and game port is 0. Used steam query port " + id + " as instanceId.",
			    ErrorExSeverity.INFO);

			s_InstanceID = id;
			return s_InstanceID;
		}

		// fallback to 0
		s_InstanceID = "0";
		ErrorEx(
		    "MetricZ instanceId is 0. Set unique 'instanceId' in server.cfg or override in 'instance_id' in metricz.json",
		    ErrorExSeverity.WARNING);

		return "0"
	}
}
#endif
