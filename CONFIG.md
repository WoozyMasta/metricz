# MetricZ Configuration

> Automatically generated list of configuration options from the source code.

This document lists configuration options used by the **MetricZ** mod
for the DayZ server.

All settings are stored in a `$profile:metricz.json` JSON configuration file

The file is created automatically with default values if it does not exist.
Changes to this file take effect after a server restart.

> [!IMPORTANT]  
> On the same host (ideally, across all servers)
> every server must have a unique `instanceId`.  
> If multiple servers run the same map (`world`), time series will collide.  
> Base labels are `{world, instance_id}`.
> Ensure `instanceId` is unique per server in `serverDZ.cfg`;
> together with the map name it forms metric identity.

## Performance & Database Recommendations

MetricZ collects highly detailed data, which can create a significant load
on your Time Series Database (TSDB). Before configuring the mod,
please read the following recommendations.

### High Cardinality & Series Churn

This mod generates metrics with unique identifiers:

* **Players:** Every new SteamID creates a new set of metric series.
* **Vehicles:** Every spawned vehicle has a unique persistence ID (Hash).

> [!WARNING]  
> On high-population servers, or servers with Traders and Virtual Garages
> (where vehicles are frequently spawned, bought, and deleted),
> this creates **"High Series Churn"**.

A standard **Prometheus** server stores indices in RAM.
High churn can lead to excessive memory usage and OOM (Out Of Memory)
crashes over time.  
If you have thousands of unique players or vehicles per month,
consider disabling specific modules:

* `disableTransportMetrics`
  (Reduces churn significantly on heavy modded servers)
* `disablePlayerMetrics`
  (If you don't need individual player vitals history)
* `disableWeaponMetrics`
  (If you have a lot of modded weapons,
  otherwise a series is created for each type)
* `disableEntityHitsMetrics` and `disableEntityKillsMetrics`
  (If you have a lot of modded weapon and ammo or modified `AttackType` AI,
  otherwise a series is created for each type)

### Recommended TSDB

For DayZ metrics, we strongly recommend using
[VictoriaMetrics](https://victoriametrics.com/)
(Single Node) instead of standard Prometheus.

* **VictoriaMetrics**
  handles high cardinality and high churn efficiently with low RAM usage.
* **Grafana Mimir**
  is also a robust option for scalability.
* **Thanos** or **Cortex**
  work well but may be overly complex for game server hosting.

### Metric Behavior

* **Lazy Initialization:**
  Metrics are not created until the first event occurs.
  For example, `weapon_shots_total` will not appear in the database
  until the first shot is fired after a restart.
* **Non-Monotonic:**
  Gauges (like player count or FPS) fluctuate up and down.
* **Resets:**
  Counters (like shots fired) reset to `0` on every server restart.
  Use the `increase()` or `rate()` functions in PromQL/MetricsQL
  to handle this automatically.

## Options [Utils/Config.c](./scripts/3_Game/MetricZ/Utils/Config.c)

* **`configVersion`** (`int`) —
  Internal config version, do not touch it
  (default: `CONFIG_VERSION`)
* **`initDelaySeconds`** (`int`) —
  Delay before the first metric collection in seconds
  (default: `60`)
* **`scrapeIntervalSeconds`** (`int`) —
  Interval between metric updates in seconds
  (default: `15`)
* **`disableRPCMetrics`** (`bool`) —
  Disable RPC metrics collection `dayz_metricz_rpc_input_total`
  (default: `false`)
* **`disableEventMetrics`** (`bool`) —
  Disable event handler metrics collection `dayz_metricz_events_total`
  (default: `false`)
* **`disablePlayerMetrics`** (`bool`) —
  Disable player-related metrics collection `dayz_metricz_player_*`
  (default: `false`)
* **`disableZombieMetrics`** (`bool`) —
  Disable zombie per-type and mind states metrics collection
  `dayz_metricz_animals_by_type`
  (default: `false`)
* **`disableAnimalMetrics`** (`bool`) —
  Disable animal per-type metrics collection `dayz_metricz_infected_by_type`
  and `dayz_metricz_infected_mind_state`
  (default: `false`)
* **`disableTransportMetrics`** (`bool`) —
  Disable vehicle and transport metrics collection
  `dayz_metricz_transport_*`
  (default: `false`)
* **`disableWeaponMetrics`** (`bool`) —
  Disable weapon per-type (count, shoot, kills and hits) metrics collection
  `dayz_metricz_weapon_shots_total`, `dayz_metricz_weapons_by_type_total`,
  `dayz_metricz_player_killed_by_total` and
  `dayz_metricz_creature_killed_by_total`
  (default: `false`)
* **`disableEntityHitsMetrics`** (`bool`) —
  Disable player and zombie/animal hit by ammo type metrics
  `dayz_metricz_player_killed_by_total` and
  `dayz_metricz_creature_killed_by_total`
  (default: `false`)
* **`entityHitDamageThreshold`** (`float`) —
  Minimum damage to log in `EEHitBy()`; -1 and less disables threshold.
  (default: `3`)
* **`entityVehicleHitDamageThreshold`** (`float`) —
  Minimum vehicle-hit damage to log in `EEHitBy()`; -1 and less disables
  threshold.
  (default: `15`)
* **`disableEntityKillsMetrics`** (`bool`) —
  Disable player and zombie/animal kill by object killer metrics
  (default: `false`)
* **`disableTerritoryMetrics`** (`bool`) —
  Disable territory flag metrics collection
  (default: `false`)
* **`disableEffectAreaMetrics`** (`bool`) —
  Disable EffectArea (Contaminated, Geyser, HotSpring, Volcanic, etc.)
  metrics
  (default: `false`)
* **`disableLocalEffectAreaMetrics`** (`bool`) —
  Disable Local EffectArea metrics like ContaminatedArea_Local created from
  Grenade_ChemGas This is disabled by default because metrics for such local
  zones will always have unique positions, thereby creating new metric
  series in the TSDB each time. Use with caution, as this may bloat your
  metrics database!
  (default: `true`)
* **`disableCoordinatesMetrics`** (`bool`) —
  Disable player and transport coordinate metrics
  (default: `false`)
* **`disableGeoCoordinatesFormat`** (`bool`) —
  Disable conversion of coordinates metrics to geo `EPSG:4326` (WGS84)
  format. By default convert position to lon/lat in `-180/180` and `-90/90`
  range. If disable, all exported coordinates hold vanilla zero relative
  meters
  (default: `false`)
* **`mapEffectiveSize`** (`float`) —
  Override effective map tiles size in world units. Useful if the web map
  size is larger than the game world size (for example, the izurvive tiles
  for Chernarus have a size of `15926`, although the world size is `15360`)
  (default: `0.0`)
