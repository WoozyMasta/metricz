# MetricZ Configuration

This document lists configuration options used by the **MetricZ** mod
for the DayZ server.

All settings are stored in a `$profile:metricz/config.json`
JSON configuration file.

The file is created automatically with default values if it does not exist.
Changes to this file take effect after a server restart.

> [!IMPORTANT]  
> On the same host (ideally, across all servers)
> every server must have a unique `instanceId`.  
> If multiple servers run the same map (`world`), time series will collide.  
> Base labels are `{world, instance_id}`.
> Ensure `instanceId` is unique per server in `serverDZ.cfg`;
> together with the map name it forms metric identity.

## File Locations

The mod generates metrics in the server profile directory.

* **Default** `$profile:metricz/export/metricz_{instance_id}.prom`  
  Standard path. Used if `file_name` is empty.
* **Custom** `$profile:metricz/export/{file_name}.prom`  
  Used if `file_name` is set in config.
* **Legacy** `$profile:metricz.prom` (_Deprecated_ ⚠️)  
  Used only if this file already exists.
  Delete it to migrate to the new folder structure.

## Export Performance Tuning

The following data is based on stress tests performed on
performance-constrained hardware to highlight the relative
impact of configuration changes.

> [!NOTE]  
> On typical production hardware, execution times are significantly lower
> (e.g., File I/O often takes < 2ms).

### File Export Strategy

* Buffering (`buffer`):
  * Recommended: `16` - `64` lines; 32 by default.
    This is the sweet spot for performance
  * Values of `0` (No buffer) significantly increase execution time
    (approx. +50% overhead) due to frequent system I/O calls.
  * Large values (`2048+`) are _slower_ than small buffers because huge
    string allocations cause memory management spikes.
* Atomicity (`atomic`):
  * Recommended: `true`.
  * Enabling atomic writes adds overhead (writes to a `.tmp` file + rename),
    taking roughly 2x longer than direct writes.
    However, disabling it allows collectors to read incomplete files,
    causing data errors. The integrity benefit outweighs the slight delay.

### HTTP Export Strategy

* Buffering (`buffer`):
  * Recommended: `64` - `512` lines; 128 by default.
  * Sending metrics in chunks reduces network overhead.
  * Value of `0` is the slowest option (approx. 2.5x slower than buffered),
    as it creates a separate HTTP request for every single metric line.

Use [MetricZ Exporter](https://github.com/WoozyMasta/metricz-exporter)
to retrieve metrics by publishing via HTTP request.

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
  Reduces churn significantly on heavy modded servers.
* `disabled_metrics.players`
  If you don't need individual player vitals history.
* `disabled_metrics.weapons`
  If you have a lot of modded weapons,
  otherwise a series is created for each type.
* `disabled_metrics.hits` / `disabled_metrics.kills`
  If you have a lot of modded weapon/ammo types.

### Recommended TSDB

For DayZ metrics, we strongly recommend using
[VictoriaMetrics](https://victoriametrics.com/)
(Single Node) instead of standard Prometheus.

* **VictoriaMetrics**
  handles high cardinality and high churn efficiently with low RAM usage.
* **Grafana Mimir**
  is also a robust option for scalability.

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

---

> Automatically generated list of configuration options from the source code.
