# MetricZ Scraped Metrics Reference

> Automatically generated list of metrics from the source code.

This document lists all metrics exposed by the **MetricZ** mod for
DayZ server. Each metric includes its identifier, type
(`GAUGE` or `COUNTER`), and description as defined in the source code.

## [Stats/Event.c](./scripts/3_Game/MetricZ/Stats/Event.c)

* **`dayz_metricz_events_total`** (`COUNTER`) —
  Total events by EventType

## [Stats/RPC.c](./scripts/3_Game/MetricZ/Stats/RPC.c)

* **`dayz_metricz_rpc_input_total`** (`COUNTER`) —
  Total input RPC calls

## [Entities/AI/AnimalStats.c](./scripts/4_World/MetricZ/Entities/AI/AnimalStats.c)

* **`dayz_metricz_animals_by_type`** (`GAUGE`) —
  Animals in world grouped by canonical type

## [Entities/AI/ZombieStats.c](./scripts/4_World/MetricZ/Entities/AI/ZombieStats.c)

* **`dayz_metricz_infected_mind_state`** (`GAUGE`) —
  Infected count by mind state
* **`dayz_metricz_infected_by_type`** (`GAUGE`) —
  Infected count by zombie type

## [Entities/Player/Metrics.c](./scripts/4_World/MetricZ/Entities/Player/Metrics.c)

* **`dayz_metricz_player_loaded`** (`GAUGE`) —
  Is player loaded (extra labels holder)
* **`dayz_metricz_player_health`** (`GAUGE`) —
  Player health 0..1
* **`dayz_metricz_player_blood`** (`GAUGE`) —
  Player blood 0..1
* **`dayz_metricz_player_shock`** (`GAUGE`) —
  Player shock 0..1
* **`dayz_metricz_player_energy`** (`GAUGE`) —
  Player energy 0..1
* **`dayz_metricz_player_water`** (`GAUGE`) —
  Player hydration 0..1
* **`dayz_metricz_player_toxicity`** (`GAUGE`) —
  Player toxicity 0..1
* **`dayz_metricz_player_temperature_celsius`** (`GAUGE`) —
  Player body temperature in celsius
* **`dayz_metricz_player_weight`** (`GAUGE`) —
  Player total weight in grams
* **`dayz_metricz_player_wetness`** (`GAUGE`) —
  Player wetness 0..1
* **`dayz_metricz_player_lifetime_seconds`** (`GAUGE`) —
  Player lifetime since spawn or load in seconds
* **`dayz_metricz_player_position_x`** (`GAUGE`) —
  Player world X
* **`dayz_metricz_player_position_y`** (`GAUGE`) —
  Player world Y
* **`dayz_metricz_player_position_z`** (`GAUGE`) —
  Player world Z
* **`dayz_metricz_player_orientation`** (`GAUGE`) —
  Player yaw degrees
* **`dayz_metricz_player_network_ping_min`** (`GAUGE`) —
  Player network ping min
* **`dayz_metricz_player_network_ping_max`** (`GAUGE`) —
  Player network ping max
* **`dayz_metricz_player_network_throttle`** (`GAUGE`) —
  Fraction of outgoing bandwidth throttled since last update 0..1
* **`dayz_metricz_player_agents_active`** (`GAUGE`) —
  Number of active disease agents affecting the player
* **`dayz_metricz_player_bleeding_active`** (`GAUGE`) —
  Number of active bleeding sources on player
* **`dayz_metricz_player_immunity_boosted`** (`GAUGE`) —
  Immunity boosted (0/1)
* **`dayz_metricz_player_unconscious`** (`GAUGE`) —
  Is unconscious (0/1)
* **`dayz_metricz_player_restrained`** (`GAUGE`) —
  Is restrained (0/1)
* **`dayz_metricz_player_third_person`** (`GAUGE`) —
  Is in third person (0/1)
* **`dayz_metricz_player_godmode`** (`GAUGE`) —
  Damage disabled (0/1)
* **`dayz_metricz_player_stat_playtime_seconds`** (`GAUGE`) —
  Analytics playtime
* **`dayz_metricz_player_stat_distance_meters`** (`GAUGE`) —
  Analytics distance
* **`dayz_metricz_player_stat_longest_survivor_hit_m`** (`GAUGE`) —
  Analytics longest survivor hit
* **`dayz_metricz_player_stat_players_killed_total`** (`COUNTER`) —
  Analytics players killed total
* **`dayz_metricz_player_stat_infected_killed_total`** (`COUNTER`) —
  Analytics infected killed total

## [Entities/Territory/Metrics.c](./scripts/4_World/MetricZ/Entities/Territory/Metrics.c)

* **`dayz_metricz_territory_lifetime`** (`GAUGE`) —
  Territory flag lifetime fraction 0..1

## [Entities/Transport/Metrics.c](./scripts/4_World/MetricZ/Entities/Transport/Metrics.c)

* **`dayz_metricz_transport_health`** (`GAUGE`) —
  Transport health 0..1
* **`dayz_metricz_transport_crew_occupied`** (`GAUGE`) —
  Number of occupied seats in transport
* **`dayz_metricz_transport_speed`** (`GAUGE`) —
  Transport speed, m/s
* **`dayz_metricz_transport_engine_on`** (`GAUGE`) —
  Engine is on (0/1)
* **`dayz_metricz_transport_fuel_fraction`** (`GAUGE`) —
  Fuel fraction 0..1
* **`dayz_metricz_transport_position_x`** (`GAUGE`) —
  Transport world X
* **`dayz_metricz_transport_position_y`** (`GAUGE`) —
  Transport world Y
* **`dayz_metricz_transport_position_z`** (`GAUGE`) —
  Transport world Z
* **`dayz_metricz_transport_orientation`** (`GAUGE`) —
  Transport yaw degrees

## [Entities/Weapons/HitStats.c](./scripts/4_World/MetricZ/Entities/Weapons/HitStats.c)

* **`dayz_metricz_player_hit_by_total`** (`COUNTER`) —
  Count of hits received by players from specific ammo types
* **`dayz_metricz_creature_hit_by_total`** (`COUNTER`) —
  Count of hits received by creatures (Zombie/Animals/eAI) from specific ammo
  types

## [Entities/Weapons/WeaponStats.c](./scripts/4_World/MetricZ/Entities/Weapons/WeaponStats.c)

* **`dayz_metricz_weapon_shots_total`** (`COUNTER`) —
  Shots by weapon base type
* **`dayz_metricz_weapons_by_type`** (`GAUGE`) —
  Weapons in world grouped by canonical type
* **`dayz_metricz_player_killed_by_total`** (`COUNTER`) —
  Count of players killed by source
* **`dayz_metricz_creature_killed_by_total`** (`COUNTER`) —
  Count of creatures (Infected/Animals/AI) killed by source

## [Storage.c](./scripts/4_World/MetricZ/Storage.c)

* **`dayz_metricz_status`** (`GAUGE`) —
  Exporter status
* **`dayz_metricz_update_duration_seconds`** (`GAUGE`) —
  Duration of last MetricZ update, seconds
* **`dayz_metricz_scrape_interval_seconds`** (`GAUGE`) —
  Configured scrape interval in seconds
* **`dayz_metricz_scrape_skipped_total`** (`COUNTER`) —
  Total scrapes skipped because a previous scrape is still running
* **`dayz_metricz_fps`** (`GAUGE`) —
  Mission updates per one second
* **`dayz_metricz_fps_window_min`** (`GAUGE`) —
  Min FPS over scrape window
* **`dayz_metricz_fps_window_max`** (`GAUGE`) —
  Max FPS over scrape window
* **`dayz_metricz_fps_window_avg`** (`GAUGE`) —
  Average FPS over scrape window
* **`dayz_metricz_fps_window_samples`** (`GAUGE`) —
  Number of 1s FPS samples in window
* **`dayz_metricz_fps_limit`** (`GAUGE`) —
  Configured FPS cap
* **`dayz_metricz_uptime_seconds`** (`GAUGE`) —
  Server uptime since start, seconds
* **`dayz_metricz_game_time_unix_seconds`** (`GAUGE`) —
  Current game Unix time seconds
* **`dayz_metricz_time_of_day`** (`GAUGE`) —
  Time of day: 0 dawn, 1 day, 2 dusk, 3 night
* **`dayz_metricz_players_online`** (`GAUGE`) —
  Total players online in the world
* **`dayz_metricz_max_players`** (`GAUGE`) —
  Configured max players
* **`dayz_metricz_infected`** (`GAUGE`) —
  Total infected in the world
* **`dayz_metricz_infected_dead_bodies`** (`GAUGE`) —
  Infected death corpses count in the world
* **`dayz_metricz_animals`** (`GAUGE`) —
  Total animals in the world
* **`dayz_metricz_animals_dead_bodies`** (`GAUGE`) —
  Animals death corpses count in the world
* **`dayz_metricz_path_graph_updates_total`** (`COUNTER`) —
  Total updates path graph regions by object in the world
* **`dayz_metricz_weapons`** (`GAUGE`) —
  Total weapons in the world
* **`dayz_metricz_suppressors`** (`GAUGE`) —
  Total weapon suppressors in the world
* **`dayz_metricz_optics`** (`GAUGE`) —
  Total item optics in the world
* **`dayz_metricz_ammo_boxes`** (`GAUGE`) —
  Total Box_Base with Ammo in the world
* **`dayz_metricz_magazines`** (`GAUGE`) —
  Total magazines in the world
* **`dayz_metricz_ammo`** (`GAUGE`) —
  Total ammo piles in the world
* **`dayz_metricz_explosives`** (`GAUGE`) —
  Total explosives (grenade, flash, smoke, claymore, plastic, improvised) in
  the world
* **`dayz_metricz_explosives_detonated_total`** (`COUNTER`) —
  Total explosives detonated (grenade, flash, smoke, claymore, plastic,
  improvised)
* **`dayz_metricz_items`** (`GAUGE`) —
  Total items (ItemBase) in the world
* **`dayz_metricz_bottles`** (`GAUGE`) —
  Total Bottle_Base (pot, cauldron, canister, etc.) in the world
* **`dayz_metricz_clothes`** (`GAUGE`) —
  Total clothes in the world
* **`dayz_metricz_boxes`** (`GAUGE`) —
  Total not Ammo Box_Base (NailBox, HeadlightH7_Box) in the world
* **`dayz_metricz_cars`** (`GAUGE`) —
  Total cars in the world
* **`dayz_metricz_boats`** (`GAUGE`) —
  Total boats in the world
* **`dayz_metricz_helicopters`** (`GAUGE`) —
  Total helicopters in the world
* **`dayz_metricz_territory_flags`** (`GAUGE`) —
  Total active bases (raised flagpole)
* **`dayz_metricz_base_buildings`** (`GAUGE`) —
  Total built player constructions (tower, fence, flag)
* **`dayz_metricz_tents`** (`GAUGE`) —
  Total tents in the world (tents and shelters)
* **`dayz_metricz_stashes`** (`GAUGE`) —
  Total buried stashes
* **`dayz_metricz_gardens`** (`GAUGE`) —
  Total garden plots
* **`dayz_metricz_containers`** (`GAUGE`) —
  Total deployable containers (crate, barrel, chest)
* **`dayz_metricz_car_parts`** (`GAUGE`) —
  Total car parts (doors & trunks)
* **`dayz_metricz_car_wheels`** (`GAUGE`) —
  Total car wheels in the world
* **`dayz_metricz_mined_bushes_total`** (`COUNTER`) —
  Count of mined bushes
* **`dayz_metricz_mined_rocks_total`** (`COUNTER`) —
  Count of mined rocks
* **`dayz_metricz_mined_trees_total`** (`COUNTER`) —
  Count of mined trees
* **`dayz_metricz_fishing_attempts_total`** (`COUNTER`) —
  Total fishing attempts
* **`dayz_metricz_fishing_catches_total`** (`COUNTER`) —
  Total fish caught
* **`dayz_metricz_corpses`** (`GAUGE`) —
  Corpses tracked on server
* **`dayz_metricz_artillery_barrages_total`** (`COUNTER`) —
  Artillery barrages on server
* **`dayz_metricz_crash_sites`** (`GAUGE`) —
  Total crash sites (mi8, uh1y, santa sleigh)
* **`dayz_metricz_gifts_unpacked_total`** (`COUNTER`) —
  Gifts unpacked on server
* **`dayz_metricz_weather_temperature`** (`GAUGE`) —
  Center of world temperature in celsius with weather factors
* **`dayz_metricz_weather_temperature_base`** (`GAUGE`) —
  Base world temperature in celsius
* **`dayz_metricz_weather_wind_speed`** (`GAUGE`) —
  Wind speed meters per second
* **`dayz_metricz_weather_wind_direction`** (`GAUGE`) —
  Wind xz angle degrees
* **`dayz_metricz_weather_rain`** (`GAUGE`) —
  Rain 0..1
* **`dayz_metricz_weather_snow`** (`GAUGE`) —
  Snow 0..1
* **`dayz_metricz_weather_fog`** (`GAUGE`) —
  Fog 0..1
* **`dayz_metricz_weather_clouds`** (`GAUGE`) —
  Clouds 0..1
* **`dayz_metricz_players_spawns_total`** (`COUNTER`) —
  Total new players spawns in the world
* **`dayz_metricz_players_deaths_total`** (`COUNTER`) —
  Total players deaths by combat
* **`dayz_metricz_infected_deaths_total`** (`COUNTER`) —
  Total infected deaths by combat
* **`dayz_metricz_animals_deaths_total`** (`COUNTER`) —
  Total animal deaths by combat
* **`dayz_metricz_cars_destroyed_total`** (`COUNTER`) —
  Total cars destroyed
* **`dayz_metricz_boats_destroyed_total`** (`COUNTER`) —
  Total boats destroyed
* **`dayz_metricz_helicopters_destroyed_total`** (`COUNTER`) —
  Total helicopters destroyed
* **`dayz_metricz_eai`** (`GAUGE`) —
  Total Expansion AI in the world (optional)
* **`dayz_metricz_eai_npc`** (`GAUGE`) —
  Total Expansion AI NPCs in the world (optional)
* **`dayz_metricz_eai_deaths_total`** (`COUNTER`) —
  Total number of Expansion AI deaths (optional)
* **`dayz_metricz_eai_npc_deaths_total`** (`COUNTER`) —
  Total number of Expansion AI NPC deaths (optional)
* **`dayz_metricz_food`** (`GAUGE`) —
  Total edible base items in the world with static labeled types
