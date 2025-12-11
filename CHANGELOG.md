# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog][],
and this project adheres to [Semantic Versioning][].

<!--
## Unreleased

### Added
### Changed
### Removed
-->

Here is the updated changelog including the new changes.

## Unreleased

### Added

* configuration has been completely moved to a new format in the JSON file ⚠️
* buffered metric export is now available in all metrics sink writers
* support for sending metrics to the proxy and aggregation backend
  [metrics-exporter](https://github.com/woozymasta/metricz-exporter)
* **`dayz_metricz_eai_deaths_total`** (`COUNTER`) —
  Total number of Expansion AI deaths (optional)
* **`dayz_metricz_eai_npc_deaths_total`** (`COUNTER`) —
  Total number of Expansion AI NPC deaths (optional)
* implemented `MetricZ_PersistentCache` to save and load unique sets of
  tags between server restarts, which will fix Prometheus rate calculations
* **`dayz_metricz_player_hit_by_total`** (`COUNTER`) —
  Count of hits received by players from specific ammo types
* **`dayz_metricz_creature_hit_by_total`** (`COUNTER`) —
  Count of hits received by creatures (Zombie/Animals/eAI) from specific ammo
  types
* **`dayz_metricz_player_killed_by_total`** (`COUNTER`) —
  Count of players killed by source
* **`dayz_metricz_creature_killed_by_total`** (`COUNTER`) —
  Count of creatures (Infected/Animals/AI) killed by source
* configuration options `disabled_metrics.hits`, `thresholds.hit_damage` and
  `thresholds.hit_damage_vehicle` to control entity hit metrics
* configuration option `disabled_metrics.kills` to disable
  player and zombie/animal kill source metrics
* debug logging of all configuration values when `DIAG` is defined
* `MetricZ_Geo` helper class for converting world coordinates to `EPSG:4326`
  (WGS84).
* new config/CLI `geo.world_effective_size` options to override map tiles size
* **`dayz_metricz_player_orientation`**  (`GAUGE`) — player yaw in degrees
* **`dayz_metricz_transport_orientation`** (`GAUGE`) — transport yaw in degrees
* **`dayz_metricz_effect_areas`** (`GAUGE`) — total active effect areas
  (static/dynamic zones)
* **`dayz_metricz_effect_area_insiders`** (`GAUGE`) —
  per-zone metrics with position and radius labels
* new config options `disabled_metrics.areas` and `disabled_metrics.local_areas`
* new config options `disabled_metrics.positions_height` and `disabled_metrics.positions_yaw`

### Changed

* sink writers have been implemented for writing metrics to disk,
  sending them over the network, or both simultaneously
* for file export, atomic write mode is now optional
* fixed an issue when trying to calculate network ping for eAI
* fixed double counting of destroyed vehicles when saving/loading
* stricter `EEKilled()` checking for objects deaths
  to prevent unnecessary counter increments
* fixed calculation of `dayz_metricz_(boats|cars|helicopters)` and
  `dayz_metricz_(boats|cars|helicopters)_destroyed_total` with installed
  Expansion Vehicles mod
* fixed calculation of `dayz_metricz_players_spawns_total`
* fixed `dayz_metricz_artillery_barrages_total` calculation logic
* removed `dayz_metricz_weapon_shots_all_total` ⚠️
* removed `dayz_metricz_player_third_person`,
  which was only available on the client side
* weapon type name for labels now use `MetricZ_ObjectName::GetName()`
* `MetricZ_ObjectName::StripSuffix()` now returns bool on success and
  accepts an `inout` type name
* `MetricZ_Config` now robustly handles missing or `0` values from
  `serverDZ.cfg` by respecting default values, while allowing explicit `-1`
  to set a value to `0` where applicable
* fixed **`dayz_metricz_artillery_barrages_total`** calculation logic
* replaced `MetricZ_EnableCoordinatesMetrics` with inverted
  `MetricZ_DisableCoordinatesMetrics` (`-metricz-disable-coordinates`) ⚠️
* coordinate metrics now use `MetricZ_Geo` and can export either raw world
  coordinates or WGS84 lon/lat based on
  `MetricZ_DisableGeoCoordinatesFormat` ⚠️
* labels for **`dayz_metricz_territory_lifetime`** changed: replaced
  `x`/`y`/`z` with `longitude`/`latitude` and added `refresher_radius` ⚠️
* documentation rendering in `CONFIG.md` has been updated and improved
* configuration reading has been normalized and brought to a unified form
* frame monitor moved to `DayZGame::OnPostUpdate()` for more stable
  calculations (if frame skipping is used by mods like GaneZ)
* spelling in panel names have been corrected

## [0.3.0][] - 2025-11-23

### Added

* **`dayz_metricz_player_network_ping_min`** (`GAUGE`) —
  player network ping min
* **`dayz_metricz_player_network_ping_max`** (`GAUGE`) —
  player network ping max
* **`dayz_metricz_player_network_throttle`** (`GAUGE`) —
  fraction of outgoing bandwidth throttled since last update 0..1
* in class `MetricZ_Exporter` moved all data writers into new
  `bool Flush(FileHandle fh)` to support modded custom metric registries

### Changed

* class `MetricZ_Exporter` rebuild to singleton instance for modding support.
  Although static overriding was introduced in version 1.29,
  static overriding does not apply when a method is called via `CallLater` ⚠️
* refactored `MetricZ_Time` and use integer as more accuracy timestamp

[0.3.0]: https://github.com/WoozyMasta/metricz/compare/0.2.1...0.3.0

## [0.2.1][] - 2025-11-16

### Changed

* removed fallback label `game_port` and used `instance_id`
  to hold game port if instance is `0`

[0.2.1]: https://github.com/WoozyMasta/metricz/compare/0.2.0...0.2.1

## [0.2.0][] - 2025-11-16

### Added

* internal labels per metric in `MetricZ_MetricBase` with helpers
  `SetLabels()`,`MakeLabels()`, `MakeLabel()` and getters
  `GetLabels()`, `GetLabelsRaw()` and `HasLabels()`
* helper class `MetricZ_ObjectName` with
  `GetName(Object obj, bool stripBase = false)` for consistent,
  normalized object type names (optional suffix stripping).
* `game_port` base label, provide the desired uniqueness when `instanceId`
  is not set in server config
* [INTEGRATION.md](./INTEGRATION.md) with info how to use MetricZ in your mods

#### New metrics

* **`dayz_metricz_animals_by_type`** (`GAUGE`) —
  Animals in world grouped by canonical type
* **`dayz_metricz_animals_dead_bodies`** (`GAUGE`) —
  Animals death corpses count in the world
* **`dayz_metricz_infected_by_type`** (`GAUGE`) —
  Infected count by zombie type
* **`dayz_metricz_infected_dead_bodies`** (`GAUGE`) —
  Infected death corpses count in the world
* **`dayz_metricz_weapons_by_type`** (`GAUGE`) —
  Weapons in world grouped by canonical type
* **`dayz_metricz_suppressors`** (`GAUGE`) —
  Total weapon suppressors in the world
* **`dayz_metricz_optics`** (`GAUGE`) —
  Total item optics in the world
* **`dayz_metricz_car_wheels`** (`GAUGE`) —
  Total car wheels in the world
* **`dayz_metricz_eai`** (`GAUGE`) —
  Total Expansion AI in the world (optional)
* **`dayz_metricz_eai_npc`** (`GAUGE`) —
  Total Expansion AI NPCs in the world (optional)

#### New options

* **`MetricZ_DisableZombieMetrics`** (`-metricz-disable-zombie`) —
  Disable zombie per-type and mind states metrics collection
* **`MetricZ_DisableAnimalMetrics`** (`-metricz-disable-animal`) —
  Disable animal per-type metrics collection

### Changed

* metric `dayz_metricz_food` extended with type-based labels:
  other, fruit, mushroom, corpse, worm, guts, meat, human_meat, disinfectant,
  pills, drink, snack, candy, canned_small, canned_medium, canned_big, jar
* class `MetricZ_ZombieMindStats` renamed to `MetricZ_ZombieStats` and now
  hold mind state and per type metrics ⚠️
* all metrics will now always have base labels if they have not been specified
* `MetricZ_LabelUtils::MakeLabels()` map with labels now is optional parameter
* `MetricZ_LabelUtils::MakeLabels()` now trim key and value, and make lower
  case and replace spaces with underscores for label key
* `MetricZ_LabelUtils::PersistentHash()` now return hash integer without
  `p` or `n` prefix ⚠️
* fixed `hash` label in transport metrics,
  previously it was not unique and changed between server restarts
* earlier initialization of the metrics store to avoid collisions
* fix not shown `blood` label on player `dayz_metricz_player_loaded` metric
* all I/O moved to separate `MetricZ_Exporter` class
* replace all `GetGame()` calls to static `g_Game` instance

[0.2.0]: https://github.com/WoozyMasta/metricz/compare/0.1.2...0.2.2

## [0.1.2][] - 2025-11-09

### Added

* [CONFIG.md](./CONFIG.md) and [METRICS.md](./METRICS.md) to workshop content
* links to github and workshop in workshop content

[0.1.2]: https://github.com/WoozyMasta/metricz/compare/0.1.1...0.1.2

## [0.1.1][] - 2025-11-03

### Added

Metrics:

* **`dayz_metricz_fps_window_min`** (`GAUGE`) —
    Min FPS over scrape window
* **`dayz_metricz_fps_window_max`** (`GAUGE`) —
    Max FPS over scrape window
* **`dayz_metricz_fps_window_avg`** (`GAUGE`) —
    Average FPS over scrape window
* **`dayz_metricz_fps_window_samples`** (`GAUGE`) —
    Number of 1s FPS samples in window

### Changed

* fix dashboard variables and panels
* added missing override
* removed redundant refs

[0.1.1]: https://github.com/WoozyMasta/metricz/compare/0.1.0...0.1.1

## [0.1.0][] - 2025-10-26

### Added

* First public release

[0.1.0]: https://github.com/WoozyMasta/metricz/tree/0.1.0

<!--links-->
[Keep a Changelog]: https://keepachangelog.com/en/1.1.0/
[Semantic Versioning]: https://semver.org/spec/v2.0.0.html
