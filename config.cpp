/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

class CfgMods
{
	class MetricZ
	{
		type = "mod";
		dir = "metricz";
		name = "Prometheus Metrics Exporter for DayZ";
		version = "0.1.0";
		credits = "WoozyMasta";
		author = "WoozyMasta";
		authorID = "76561198037610867";
		hideName = 1;
		hidePicture = 1;
		defines[] = {"METRICZ"};
		dependencies[] = {"game", "world", "mission"};

		class defs
		{
			class gameScriptModule
			{
				files[] = {"metricz/scripts/3_game"};
			};

			class worldScriptModule
			{
				files[] = { "metricz/scripts/4_world" };
			};

			class missionScriptModule
			{
				files[] = { "metricz/scripts/5_mission" };
			};
		};
	};
};

class CfgPatches
{
	class MetricZ
	{
		requiredAddons[] = {
			"DZ_Data",
			"DZ_Scripts",
		};
	};
};
