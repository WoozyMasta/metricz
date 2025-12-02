/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief Global storage and updater for world-level metrics.
    \details Owns metric instances, labels, and flush logic for non-player metrics.
*/
class MetricZ_Storage
{
	// init flag
	protected static bool s_Initialized;

	// metrics cached labels
	protected static string s_Labels;
	protected static string s_LabelsExtra;

	// Metrics storage registry
	protected static ref array<ref MetricZ_MetricBase> s_Registry = new array<ref MetricZ_MetricBase>();

	// Food metrics map: enum -> metric
	protected static ref map<MetricZ_FoodTypes, ref MetricZ_MetricInt> s_FoodMetricByType;

	// --- Gauges
	// Core
	static ref MetricZ_MetricInt s_Status = new MetricZ_MetricInt(
	    "status",
	    "Exporter status",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat s_UpdateDurationSec = new MetricZ_MetricFloat(
	    "update_duration_seconds",
	    "Duration of last MetricZ update, seconds",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_ScrapeInterval = new MetricZ_MetricInt(
	    "scrape_interval_seconds",
	    "Configured scrape interval in seconds",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_ScrapeSkippedTotal = new MetricZ_MetricInt(
	    "scrape_skipped",
	    "Total scrapes skipped because a previous scrape is still running",
	    MetricZ_MetricType.COUNTER);

	// FPS
	static ref MetricZ_MetricFloat s_FPS = new MetricZ_MetricFloat(
	    "fps",
	    "Mission updates per one second",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat s_FPSMin = new MetricZ_MetricFloat(
	    "fps_window_min",
	    "Min FPS over scrape window",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat s_FPSMax = new MetricZ_MetricFloat(
	    "fps_window_max",
	    "Max FPS over scrape window",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat s_FPSAvg = new MetricZ_MetricFloat(
	    "fps_window_avg",
	    "Average FPS over scrape window",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_FPSSamples = new MetricZ_MetricInt(
	    "fps_window_samples",
	    "Number of 1s FPS samples in window",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_FPSLimit = new MetricZ_MetricInt(
	    "fps_limit",
	    "Configured FPS cap",
	    MetricZ_MetricType.GAUGE);

	// Time
	static ref MetricZ_MetricFloat s_ServerUptimeSec = new MetricZ_MetricFloat(
	    "uptime_seconds",
	    "Server uptime since start, seconds",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_TimeUnixSec = new MetricZ_MetricInt(
	    "game_time_unix_seconds",
	    "Current game Unix time seconds",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_TimeOfDay = new MetricZ_MetricInt(
	    "time_of_day",
	    "Time of day: 0 dawn, 1 day, 2 dusk, 3 night",
	    MetricZ_MetricType.GAUGE);

	// World totals
	static ref MetricZ_MetricInt s_PlayersOnline = new MetricZ_MetricInt(
	    "players_online",
	    "Total players online in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_MaxPlayers = new MetricZ_MetricInt(
	    "max_players",
	    "Configured max players",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Infected = new MetricZ_MetricInt(
	    "infected",
	    "Total infected in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_InfectedCorpses = new MetricZ_MetricInt(
	    "infected_dead_bodies",
	    "Infected death corpses count in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Animals = new MetricZ_MetricInt(
	    "animals",
	    "Total animals in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_AnimalsCorpses = new MetricZ_MetricInt(
	    "animals_dead_bodies",
	    "Animals death corpses count in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_PathGraphUpdates = new MetricZ_MetricInt(
	    "path_graph_updates",
	    "Total updates path graph regions by object in the world",
	    MetricZ_MetricType.COUNTER);

	// Weapon related
	static ref MetricZ_MetricInt s_Weapons = new MetricZ_MetricInt(
	    "weapons",
	    "Total weapons in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Suppressors = new MetricZ_MetricInt(
	    "suppressors",
	    "Total weapon suppressors in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Optics = new MetricZ_MetricInt(
	    "optics",
	    "Total item optics in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_AmmoBoxes = new MetricZ_MetricInt(
	    "ammo_boxes",
	    "Total Box_Base with Ammo in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Magazines = new MetricZ_MetricInt(
	    "magazines",
	    "Total magazines in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Ammo = new MetricZ_MetricInt(
	    "ammo",
	    "Total ammo piles in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Explosives = new MetricZ_MetricInt(
	    "explosives",
	    "Total explosives (grenade, flash, smoke, claymore, plastic, improvised) in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Exploded = new MetricZ_MetricInt(
	    "explosives_detonated",
	    "Total explosives detonated (grenade, flash, smoke, claymore, plastic, improvised)",
	    MetricZ_MetricType.COUNTER);

	// Items
	static ref MetricZ_MetricInt s_Items = new MetricZ_MetricInt(
	    "items",
	    "Total items (ItemBase) in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Bottles = new MetricZ_MetricInt(
	    "bottles",
	    "Total Bottle_Base (pot, cauldron, canister, etc.) in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Clothing = new MetricZ_MetricInt(
	    "clothes",
	    "Total clothes in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Boxes = new MetricZ_MetricInt(
	    "boxes",
	    "Total not Ammo Box_Base (NailBox, HeadlightH7_Box) in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Cars = new MetricZ_MetricInt(
	    "cars",
	    "Total cars in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Boats = new MetricZ_MetricInt(
	    "boats",
	    "Total boats in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Helicopters = new MetricZ_MetricInt(
	    "helicopters",
	    "Total helicopters in the world",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_TerritoryFlags = new MetricZ_MetricInt(
	    "territory_flags",
	    "Total active bases (raised flagpole)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_BaseBuildings = new MetricZ_MetricInt(
	    "base_buildings",
	    "Total built player constructions (tower, fence, flag)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Tents = new MetricZ_MetricInt(
	    "tents",
	    "Total tents in the world (tents and shelters)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Stashes = new MetricZ_MetricInt(
	    "stashes",
	    "Total buried stashes",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Gardens = new MetricZ_MetricInt(
	    "gardens",
	    "Total garden plots",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Containers = new MetricZ_MetricInt(
	    "containers",
	    "Total deployable containers (crate, barrel, chest)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_CarParts = new MetricZ_MetricInt(
	    "car_parts",
	    "Total car parts (doors & trunks)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_CarWheels = new MetricZ_MetricInt(
	    "car_wheels",
	    "Total car wheels in the world",
	    MetricZ_MetricType.GAUGE);

	// Food
	static ref MetricZ_MetricInt s_Food;
	static ref MetricZ_MetricInt s_FoodOther;
	static ref MetricZ_MetricInt s_FoodFruit;
	static ref MetricZ_MetricInt s_FoodMushroom;
	static ref MetricZ_MetricInt s_FoodCorpse;
	static ref MetricZ_MetricInt s_FoodWorm;
	static ref MetricZ_MetricInt s_FoodGuts;
	static ref MetricZ_MetricInt s_FoodMeat;
	static ref MetricZ_MetricInt s_FoodHumanMeat;
	static ref MetricZ_MetricInt s_FoodDisinfectant;
	static ref MetricZ_MetricInt s_FoodPills;
	static ref MetricZ_MetricInt s_FoodDrink;
	static ref MetricZ_MetricInt s_FoodSnack;
	static ref MetricZ_MetricInt s_FoodCandy;
	static ref MetricZ_MetricInt s_FoodCannedSmall;
	static ref MetricZ_MetricInt s_FoodCannedMedium;
	static ref MetricZ_MetricInt s_FoodCannedBig;
	static ref MetricZ_MetricInt s_FoodCannedJar;

	// Mining
	static ref MetricZ_MetricInt s_MinedBushes = new MetricZ_MetricInt(
	    "mined_bushes",
	    "Count of mined bushes",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_MinedRocks = new MetricZ_MetricInt(
	    "mined_rocks",
	    "Count of mined rocks",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_MinedTrees = new MetricZ_MetricInt(
	    "mined_trees",
	    "Count of mined trees",
	    MetricZ_MetricType.COUNTER);

	// Fishing
	static ref MetricZ_MetricInt s_FishingAttempts = new MetricZ_MetricInt(
	    "fishing_attempts",
	    "Total fishing attempts",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_FishingCatches = new MetricZ_MetricInt(
	    "fishing_catches",
	    "Total fish caught",
	    MetricZ_MetricType.COUNTER);

	// Events
	static ref MetricZ_MetricInt s_Corpses = new MetricZ_MetricInt(
	    "corpses",
	    "Corpses tracked on server",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_Artillery = new MetricZ_MetricInt(
	    "artillery_barrages",
	    "Artillery barrages on server",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_CrashSites = new MetricZ_MetricInt(
	    "crash_sites",
	    "Total crash sites (mi8, uh1y, santa sleigh)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_GiftsUnpacked = new MetricZ_MetricInt(
	    "gifts_unpacked",
	    "Gifts unpacked on server",
	    MetricZ_MetricType.COUNTER);

	// Weather
	static ref MetricZ_MetricFloat m_Temperature = new MetricZ_MetricFloat(
	    "weather_temperature",
	    "Center of world temperature in celsius with weather factors",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat m_TemperatureBase = new MetricZ_MetricFloat(
	    "weather_temperature_base",
	    "Base world temperature in celsius",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat m_WindSpeed = new MetricZ_MetricFloat(
	    "weather_wind_speed",
	    "Wind speed meters per second",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat m_WindDirection = new MetricZ_MetricFloat(
	    "weather_wind_direction",
	    "Wind xz angle degrees",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat m_Rain = new MetricZ_MetricFloat(
	    "weather_rain",
	    "Rain 0..1",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat m_Snow = new MetricZ_MetricFloat(
	    "weather_snow",
	    "Snow 0..1",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat m_Fog = new MetricZ_MetricFloat(
	    "weather_fog",
	    "Fog 0..1",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricFloat m_Clouds = new MetricZ_MetricFloat(
	    "weather_clouds",
	    "Clouds 0..1",
	    MetricZ_MetricType.GAUGE);

	// --- Counters
	static ref MetricZ_MetricInt s_PlayersSpawns = new MetricZ_MetricInt(
	    "players_spawns",
	    "Total new players spawns in the world",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_PlayersDeaths = new MetricZ_MetricInt(
	    "players_deaths",
	    "Total players deaths by combat",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_InfectedDeaths = new MetricZ_MetricInt(
	    "infected_deaths",
	    "Total infected deaths by combat",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_AnimalsDeaths = new MetricZ_MetricInt(
	    "animals_deaths",
	    "Total animal deaths by combat",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_CarsDestroys = new MetricZ_MetricInt(
	    "cars_destroyed",
	    "Total cars destroyed",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_BoatsDestroys = new MetricZ_MetricInt(
	    "boats_destroyed",
	    "Total boats destroyed",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_HelicoptersDestroys = new MetricZ_MetricInt(
	    "helicopters_destroyed",
	    "Total helicopters destroyed",
	    MetricZ_MetricType.COUNTER);

#ifdef EXPANSIONMODAI
	static ref MetricZ_MetricInt s_ExpansionAI = new MetricZ_MetricInt(
	    "eai",
	    "Total Expansion AI in the world (optional)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_ExpansionAINPC = new MetricZ_MetricInt(
	    "eai_npc",
	    "Total Expansion AI NPCs in the world (optional)",
	    MetricZ_MetricType.GAUGE);
	static ref MetricZ_MetricInt s_ExpansionAIDeaths = new MetricZ_MetricInt(
	    "eai_deaths",
	    "Total number of Expansion AI deaths (optional)",
	    MetricZ_MetricType.COUNTER);
	static ref MetricZ_MetricInt s_ExpansionAINPCDeaths = new MetricZ_MetricInt(
	    "eai_npc_deaths",
	    "Total number of Expansion AI NPC deaths (optional)",
	    MetricZ_MetricType.COUNTER);
#endif

	/**
	    \brief One-time initialization and label build.
	    \details Populates registry and sets status=1.
	*/
	static void Init()
	{
		if (s_Initialized)
			return;

		// Core
		s_Registry.Insert(s_Status);
		s_Registry.Insert(s_ScrapeInterval);
		s_Registry.Insert(s_UpdateDurationSec);
		s_Registry.Insert(s_ScrapeSkippedTotal);

		// FPS
		s_Registry.Insert(s_FPS);
		s_Registry.Insert(s_FPSAvg);
		s_Registry.Insert(s_FPSMin);
		s_Registry.Insert(s_FPSMax);
		s_Registry.Insert(s_FPSSamples);
		s_Registry.Insert(s_FPSLimit);

		// Time
		s_Registry.Insert(s_ServerUptimeSec);
		s_Registry.Insert(s_TimeUnixSec);
		s_Registry.Insert(s_TimeOfDay);

		// World totals
		s_Registry.Insert(s_PlayersOnline);
		s_Registry.Insert(s_MaxPlayers);
		s_Registry.Insert(s_Infected);
		s_Registry.Insert(s_InfectedCorpses);
		s_Registry.Insert(s_Animals);
		s_Registry.Insert(s_AnimalsCorpses);
		s_Registry.Insert(s_PathGraphUpdates);

		// Weapon related
		s_Registry.Insert(s_Weapons);
		s_Registry.Insert(s_Suppressors);
		s_Registry.Insert(s_Optics);
		s_Registry.Insert(s_AmmoBoxes);
		s_Registry.Insert(s_Magazines);
		s_Registry.Insert(s_Ammo);
		s_Registry.Insert(s_Explosives);
		s_Registry.Insert(s_Exploded);

		// Items
		s_Registry.Insert(s_Items);
		s_Registry.Insert(s_Bottles);
		s_Registry.Insert(s_Clothing);
		s_Registry.Insert(s_Boxes);
		s_Registry.Insert(s_Cars);
		s_Registry.Insert(s_Boats);
		s_Registry.Insert(s_Helicopters);
		s_Registry.Insert(s_TerritoryFlags);
		s_Registry.Insert(s_BaseBuildings);
		s_Registry.Insert(s_Tents);
		s_Registry.Insert(s_Stashes);
		s_Registry.Insert(s_Gardens);
		s_Registry.Insert(s_Containers);
		s_Registry.Insert(s_CarParts);
		s_Registry.Insert(s_CarWheels);

		// Food
		InitFoodMetric();

		// Mining
		s_Registry.Insert(s_MinedBushes);
		s_Registry.Insert(s_MinedRocks);
		s_Registry.Insert(s_MinedTrees);

		// Fishing
		s_Registry.Insert(s_FishingAttempts);
		s_Registry.Insert(s_FishingCatches);

		// Events
		s_Registry.Insert(s_Corpses);
		s_Registry.Insert(s_Artillery);
		s_Registry.Insert(s_CrashSites);
		s_Registry.Insert(s_GiftsUnpacked);

		// Weather
		s_Registry.Insert(m_Temperature);
		s_Registry.Insert(m_TemperatureBase);
		s_Registry.Insert(m_WindSpeed);
		s_Registry.Insert(m_WindDirection);
		s_Registry.Insert(m_Rain);
		s_Registry.Insert(m_Snow);
		s_Registry.Insert(m_Fog);
		s_Registry.Insert(m_Clouds);

		// Counters
		s_Registry.Insert(s_PlayersSpawns);
		s_Registry.Insert(s_PlayersDeaths);
		s_Registry.Insert(s_InfectedDeaths);
		s_Registry.Insert(s_AnimalsDeaths);
		s_Registry.Insert(s_CarsDestroys);
		s_Registry.Insert(s_BoatsDestroys);
		s_Registry.Insert(s_HelicoptersDestroys);

#ifdef EXPANSIONMODAI
		s_Registry.Insert(s_ExpansionAI);
		s_Registry.Insert(s_ExpansionAINPC);
		s_Registry.Insert(s_ExpansionAIDeaths);
		s_Registry.Insert(s_ExpansionAINPCDeaths);
#endif

		SetLabels();

		s_Status.Set(1);
		s_ScrapeInterval.Set(MetricZ_Config.s_ScrapeIntervalMs / 1000);
		s_ScrapeSkippedTotal.Set(0);
		s_FPSLimit.Set(MetricZ_Config.s_LimitFPS);
		s_MaxPlayers.Set(MetricZ_Config.s_MaxPlayers);

		s_Initialized = true;
	}

	/**
	    \brief Build common label set for status metric.
	    \details world, host, game_version, instance_id.
	*/
	static void SetLabels()
	{
		map<string, string> labels = new map<string, string>();

		s_Labels = MetricZ_LabelUtils.MakeLabels(labels);

		string game_version;
		g_Game.GetVersion(game_version);
		game_version.TrimInPlace();
		game_version.ToLower();

		labels.Insert("game_version", game_version);
		labels.Insert("save_version", g_Game.SaveVersion().ToString());

		s_LabelsExtra = MetricZ_LabelUtils.MakeLabels(labels);
	}

	/**
	    \brief Refresh metric values from game state.
	*/
	static void Update()
	{
		Init();

		// FPS
		s_FPS.Set(MetricZ_FrameMonitor.GetFPS());

		// FPS window stats for current scrape interval
		float fpsMin, fpsMax, fpsAvg;
		int fpsSamples;
		MetricZ_FrameMonitor.SnapshotWindow(fpsMin, fpsMax, fpsAvg, fpsSamples);
		s_FPSMin.Set(fpsMin);
		s_FPSMax.Set(fpsMax);
		s_FPSAvg.Set(fpsAvg);
		s_FPSSamples.Set(fpsSamples);

		// Time
		s_ServerUptimeSec.Set(g_Game.GetTickTime());
		s_TimeUnixSec.Set(MetricZ_Time.GameEpochSeconds());

		// remap to ordered: 0 DAWN, 1 DAY, 2 DUSK, 3 NIGHT
		int phase;
		switch (g_Game.GetMission().GetWorldData().GetDaytime()) {
		case WorldDataDaytime.DAWN:
			phase = 0;
			break;

		case WorldDataDaytime.DAY:
			phase = 1;
			break;

		case WorldDataDaytime.DUSK:
			phase = 2;
			break;

		case WorldDataDaytime.NIGHT:
		default:
			phase = 3;
			break;
		}
		s_TimeOfDay.Set(phase);

		// Players
		array<Man> mans = new array<Man>();
		g_Game.GetPlayers(mans);
		s_PlayersOnline.Set(mans.Count());

		// Path Graph
		s_PathGraphUpdates.Set(g_Game.s_MetricZ_PathGraphUpdates);

		// World temperature
		m_Temperature.Set(MetricZ_WorldTemperature.Get());
		m_TemperatureBase.Set(g_Game.GetMission().GetWorldData().GetBaseEnvTemperature());

		// Weather
		Weather w = g_Game.GetWeather();
		if (w) {
			m_WindSpeed.Set(w.GetWindSpeed());

			float windDirDeg = w.WindDirectionToAngle(w.GetWind()) * Math.RAD2DEG;
			if (windDirDeg < 0)
				windDirDeg += 360;
			m_WindDirection.Set(windDirDeg);

			m_Rain.Set(w.GetRain().GetActual());
			m_Snow.Set(w.GetSnowfall().GetActual());
			m_Fog.Set(w.GetFog().GetActual());
			m_Clouds.Set(w.GetOvercast().GetActual());
		}
	}

	/**
	    \brief Get common label set for most world metrics.
	    \return \p string Prebuilt label like "{world=\"...\",host=\"...\",instance_id=\"...\"}"
	*/
	static string GetLabels()
	{
		return s_Labels;
	}

	/**
	    \brief Get extended label set for status-like metrics.
	    \details Includes base labels plus version fields.
	    \return \p string Prebuilt label like
	            "{world=\"...\",host=\"...\",instance_id=\"...\",game_version=\"...\",save_version=\"...\"}"
	*/
	static string GetExtraLabels()
	{
		return s_LabelsExtra;
	}

	/**
	    \brief Flush all registered metrics.
	    \param fh Open file handle
	*/
	static void Flush(FileHandle fh)
	{
		if (!s_Initialized || s_Registry.Count() < 1)
			return;

		foreach (MetricZ_MetricBase metric : s_Registry) {
			if (metric == s_Status)
				metric.FlushWithHead(fh, s_LabelsExtra);
			else if (metric.HasLabels())
				metric.Flush(fh);
			else
				metric.FlushWithHead(fh, s_Labels);
		}
	}

	/**
	    \brief Adjust per-type food metric on spawn/delete.
	    \details
	      - Ignores NONE (untracked / skipped types).
	      - If a typed metric exists in s_FoodMetricByType, increments/decrements it.
	      - Otherwise falls back to global s_Food gauge.
	    \param foodType MetricZ_FoodTypes bucket for this Edible_Base.
	    \param increase true on spawn, false on delete.
	*/
	static void FoodMetricChange(MetricZ_FoodTypes foodType, bool increase)
	{
		if (foodType == MetricZ_FoodTypes.NONE)
			return;

		MetricZ_MetricInt metric;
		if (s_FoodMetricByType && s_FoodMetricByType.Find(foodType, metric)) {
			if (increase)
				metric.Inc();
			else
				metric.Dec();

			return;
		}

		if (!s_Food)
			return;

		if (increase)
			s_Food.Inc();
		else
			s_Food.Dec();
	}

	/**
	    \brief Factory for food metrics with static type label.
	    \details
	      - Always creates a "food" gauge metric.
	      - For NONE returns unlabeled total "food" metric.
	      - For other types attaches "food_type=<enum name>" label.
	    \param foodType MetricZ_FoodTypes bucket for this metric instance.
	    \return \p MetricZ_MetricInt New metric instance.
	*/
	protected static MetricZ_MetricInt NewFoodMetric(MetricZ_FoodTypes foodType)
	{
		MetricZ_MetricInt metric = new MetricZ_MetricInt(
		    "food",
		    "Total edible base items in the world with static labeled types",
		    MetricZ_MetricType.GAUGE);

		if (foodType == MetricZ_FoodTypes.NONE)
			return metric;

		string labelValue = EnumTools.EnumToString(MetricZ_FoodTypes, foodType);
		labelValue.ToLower();
		metric.MakeLabel("food_type", labelValue);

		return metric;
	}

	/**
	    \brief Initialize typed food metrics registry once.
	    \details
	      - Registers global s_Food metric.
	      - Registers each typed food metric in s_Registry.
	      - Fills s_FoodMetricByType map: MetricZ_FoodTypes -> MetricZ_MetricInt.
	      Safe to call multiple times.
	*/
	protected static void InitFoodMetric()
	{
		if (s_FoodMetricByType)
			return;

		s_FoodMetricByType = new map<MetricZ_FoodTypes, ref MetricZ_MetricInt>();

		s_Food = NewFoodMetric(MetricZ_FoodTypes.NONE);
		s_Registry.Insert(s_Food);

		s_FoodOther = NewFoodMetric(MetricZ_FoodTypes.OTHER);
		s_Registry.Insert(s_FoodOther);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.OTHER, s_FoodOther);

		s_FoodFruit = NewFoodMetric(MetricZ_FoodTypes.FRUIT);
		s_Registry.Insert(s_FoodFruit);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.FRUIT, s_FoodFruit);

		s_FoodMushroom = NewFoodMetric(MetricZ_FoodTypes.MUSHROOM);
		s_Registry.Insert(s_FoodMushroom);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.MUSHROOM, s_FoodMushroom);

		s_FoodCorpse = NewFoodMetric(MetricZ_FoodTypes.CORPSE);
		s_Registry.Insert(s_FoodCorpse);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.CORPSE, s_FoodCorpse);

		s_FoodWorm = NewFoodMetric(MetricZ_FoodTypes.WORM);
		s_Registry.Insert(s_FoodWorm);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.WORM, s_FoodWorm);

		s_FoodGuts = NewFoodMetric(MetricZ_FoodTypes.GUTS);
		s_Registry.Insert(s_FoodGuts);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.GUTS, s_FoodGuts);

		s_FoodMeat = NewFoodMetric(MetricZ_FoodTypes.MEAT);
		s_Registry.Insert(s_FoodMeat);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.MEAT, s_FoodMeat);

		s_FoodHumanMeat = NewFoodMetric(MetricZ_FoodTypes.HUMAN_MEAT);
		s_Registry.Insert(s_FoodHumanMeat);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.HUMAN_MEAT, s_FoodHumanMeat);

		s_FoodDisinfectant = NewFoodMetric(MetricZ_FoodTypes.DISINFECTANT);
		s_Registry.Insert(s_FoodDisinfectant);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.DISINFECTANT, s_FoodDisinfectant);

		s_FoodPills = NewFoodMetric(MetricZ_FoodTypes.PILLS);
		s_Registry.Insert(s_FoodPills);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.PILLS, s_FoodPills);

		s_FoodDrink = NewFoodMetric(MetricZ_FoodTypes.DRINK);
		s_Registry.Insert(s_FoodDrink);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.DRINK, s_FoodDrink);

		s_FoodSnack = NewFoodMetric(MetricZ_FoodTypes.SNACK);
		s_Registry.Insert(s_FoodSnack);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.SNACK, s_FoodSnack);

		s_FoodCandy = NewFoodMetric(MetricZ_FoodTypes.CANDY);
		s_Registry.Insert(s_FoodCandy);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.CANDY, s_FoodCandy);

		s_FoodCannedSmall = NewFoodMetric(MetricZ_FoodTypes.CANNED_SMALL);
		s_Registry.Insert(s_FoodCannedSmall);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.CANNED_SMALL, s_FoodCannedSmall);

		s_FoodCannedMedium = NewFoodMetric(MetricZ_FoodTypes.CANNED_MEDIUM);
		s_Registry.Insert(s_FoodCannedMedium);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.CANNED_MEDIUM, s_FoodCannedMedium);

		s_FoodCannedBig = NewFoodMetric(MetricZ_FoodTypes.CANNED_BIG);
		s_Registry.Insert(s_FoodCannedBig);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.CANNED_BIG, s_FoodCannedBig);

		s_FoodCannedJar = NewFoodMetric(MetricZ_FoodTypes.JAR);
		s_Registry.Insert(s_FoodCannedJar);
		s_FoodMetricByType.Insert(MetricZ_FoodTypes.JAR, s_FoodCannedJar);
	}
}
#endif
