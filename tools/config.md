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

* `disabled_metrics.transports`
  (Reduces churn significantly on heavy modded servers)
* `disabled_metrics.players`
  (If you don't need individual player vitals history)
* `disabled_metrics.weapons`
  (If you have a lot of modded weapons,
  otherwise a series is created for each type)
* `disabled_metrics.hits` and `disabled_metrics.kills`
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
