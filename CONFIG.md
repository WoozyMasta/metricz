# MetricZ Configuration

> Automatically generated list of configuration options from the source code.

This document lists configuration options and CLI flags
used by the **MetricZ** mod for DayZ server.

Each entry below shows:

1. the **serverDZ.cfg** option (with `MetricZ_` prefix)
2. the equivalent **CLI flag** (with `-metricz-` prefix)
3. a short description taken from the source code comments.

## Performance & Database Recommendations

MetricZ collects highly detailed data, which can create a significant load
on your Time Series Database (TSDB). Before configuring the mod,
please read the following recommendations.

### High Cardinality & Series Churn

This mod generates metrics with unique identifiers:

* **Players:** Every new SteamID creates a new set of metric series.
* **Vehicles:** Every spawned vehicle has a unique persistence ID (Hash).

On high-population servers, or servers with Traders and Virtual Garages
(where vehicles are frequently spawned, bought, and deleted),
this creates **"High Series Churn"**.

A standard **Prometheus** server stores indices in RAM.
High churn can lead to excessive memory usage and OOM (Out Of Memory)
crashes over time.  
If you have thousands of unique players or vehicles per month,
consider disabling specific modules:

* `-metricz-disable-transport`
  (Reduces churn significantly on heavy modded servers)
* `-metricz-disable-player`
  (If you don't need individual player vitals history)
* `-metricz-disable-weapon`
  (If you have a lot of modded weapons,
  otherwise a series is created for each type)
* `-metricz-disable-entity-hits`
  (If you have a lot of modded ammo or modified `AttackType` AI,
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

## Notes

* CLI flags override configuration file values.
* Boolean options accept:
  * `true` / `1` — enable feature
  * `false` / `0` — disable feature
* Time values are specified in **seconds** in both config and CLI.
* If a minimum is defined, lower values will be clamped.
* String parameters can only be passed in CLI flags.

## Options [Utils/Config.c](./scripts/3_Game/MetricZ/Utils/Config.c)

* **`MetricZ_InitDelay`**
  `-metricz-init-delay`
  (default: `60`)
  — Delay before the first metric collection in seconds
* **`MetricZ_ScrapeInterval`**
  `-metricz-scrape-interval`
  (default: `15`)
  — Interval between metric updates in seconds
* **`MetricZ_DisablePlayerMetrics`**
  `-metricz-disable-player`
  — Disable player-related metrics collection
* **`MetricZ_DisableZombieMetrics`**
  `-metricz-disable-zombie`
  — Disable zombie per-type and mind states metrics collection
* **`MetricZ_DisableAnimalMetrics`**
  `-metricz-disable-animal`
  — Disable animal per-type metrics collection
* **`MetricZ_DisableTransportMetrics`**
  `-metricz-disable-transport`
  — Disable vehicle and transport metrics collection
* **`MetricZ_DisableWeaponMetrics`**
  `-metricz-disable-weapon`
  — Disable weapon per-type (count, shoot and kills) metrics collection
* **`MetricZ_DisableEntityHitsMetrics`**
  `-metricz-disable-entity-hits`
  — Disable player and zombie/animal hit by ammo type metrics
* **`MetricZ_EntityHitDamageThreshold`**
  `-metricz-entity-hit-damage-threshold`
  (default: `3`)
  — Minimum damage to log in `EEHitBy()`; -1 and less disables threshold.
* **`MetricZ_EntityVehicleHitDamageThreshold`**
  `-metricz-entity-vehicle-hit-damage-threshold`
  (default: `15`)
  — Minimum vehicle-hit damage to log in `EEHitBy()`; -1 and less disables
 threshold.
* **`MetricZ_DisableTerritoryMetrics`**
  `-metricz-disable-territory`
  — Disable territory flag metrics collection
* **`MetricZ_EnableCoordinatesMetrics`**
  `-metricz-enable-coordinates`
  — Enable player and transport coordinate metrics
* **`MetricZ_DisableRPCMetrics`**
  `-metricz-disable-rpc`
  — Disable RPC metrics collection
* **`MetricZ_DisableEventMetrics`**
  `-metricz-disable-event`
  — Disable event handler metrics collection
