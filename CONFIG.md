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
> (e.g., File I/O often takes ~1ms, HTTP Serialized export ~0ms).

### File Export Strategy

**Buffering** (`buffer`):

* Recommended: `16` - `64` lines; 32 by default.
  This is the sweet spot for performance
* Values of `0` (No buffer) significantly increase execution time
  (approx. +50% overhead) due to frequent system I/O calls.
* Large values (`2048+`) are _slower_ than small buffers because huge
  string allocations cause memory management spikes.

**Atomicity** (`atomic`):

* Recommended: `true`.
* Enabling atomic writes adds overhead (writes to a `.tmp` file + rename),
  taking roughly 2x longer than direct writes.
  However, disabling it allows collectors to read incomplete files,
  causing data errors. The integrity benefit outweighs the slight delay.

### HTTP Export Strategy

**Serialization** (`serialized`):

* Recommended: `true` (Default).
* Switches the payload format to a JSON array of strings.
* Performance: This uses the game engine's native C++ JSON serializer,
  bypassing the slow string concatenation of the scripting language.
  * Serialized (JSON): ~4ms (Fastest).
  * Text Buffered: ~9ms (~2x slower).
  * Text Unbuffered: ~30ms (~6x slower).

**Buffering** (`buffer`):

* With Serialization (`true`):
  * Recommended: `-1` (Unlimited).
  * Since serialization is handled natively, creating one large request
    is extremely efficient.
    Chunking is only necessary if you hit network payload size limits.
* Without Serialization (`false`):
  * Recommended: `64` - `512` lines.
  * Necessary to avoid freezing the server.
    Sending all metrics in one text blob causes massive CPU spikes
    due to string manipulation overhead.

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

## Options [Config/DTO.c](./scripts/3_Game/MetricZ/Config/DTO.c)

### Config

* **`version`** (`string`) = MetricZ_Constants.VERSION -
  Internal configuration version. **Do not modify**.
* **`settings`** (`ref MetricZ_ConfigDTO_BaseSettings`) -
  Base settings for metric collection.
* **`file`** (`ref MetricZ_ConfigDTO_FileExport`) -
  Settings for exporting metrics to a local file.
* **`http`** (`ref MetricZ_ConfigDTO_HttpExport`) -
  Settings for publishing metrics via HTTP.
* **`disabled_metrics`** (`ref MetricZ_ConfigDTO_DisabledMetrics`) -
  Switches to disable specific metric series.
* **`thresholds`** (`ref MetricZ_ConfigDTO_Thresholds`) -
  Metric collection thresholds.
* **`geo`** (`ref MetricZ_ConfigDTO_Geo`) -
  Geographic coordinate settings.

### BaseSettings

* **`settings.instance_id`** (`string`) -
  Overrides the Instance ID. By default, this is detected automatically from
  serverDZ.cfg instanceID, or uses gamePort/steamQueryPort if instanceID is
  undefined or zero. This ID must be unique across all game servers on all
  hosts. You can override it here to prevent storage and profile path
  changes on existing servers.
* **`settings.init_delay_sec`** (`int`) = 30 -
  Delay in seconds before the first metric collection begins.
* **`settings.collect_interval_sec`** (`int`) = 15 -
  Interval in seconds between metric updates.

### FileExport

* **`file.enabled`** (`bool`) = true -
  Enables saving metrics to a local file (`metricz.prom`) for collection by
  node-exporter or windows-exporter textfile collector.
* **`file.buffer`** (`int`) = 32 -
  Buffer size (in lines) for file writing. 0 - Disable buffer, write each
  line to the file separately. Positive value - Write to file buffered by
  line count (recommended). Negative value - Use unlimited buffer size,
  write the whole buffer at the end (not recommended). Recommended range:
  16-64 for optimal performance. This ensures low I/O but maintains a
  reasonable buffer. Values of 2048 or larger cause huge memory allocations
  and can drop performance.
* **`file.atomic`** (`bool`) = true -
  Enables atomic export. Metrics are written to a temporary file and
  copied/renamed to the .prom file upon completion, removing the temp file.
  This adds extra I/O overhead and takes nearly x2-3 as long to write, but
  it is crucial to prevent the collector from reading incomplete files.
  Disabling this is not recommended.
* **`file.file_name`** (`string`) -
  File name override (without extension) Default file path
  `$profile:metricz/export/metricz_${instance_id}.prom`. File path with
  override `$profile:metricz/export/${file_name}.prom`.
* **`file.delete_on_shutdown`** (`bool`) -
  Delete the PROM file when shutting down the server. This is not
  recommended to be enabled by default, since upon completion of work, a
  single metric `dayz_metricz_status=0` is written to the file to indicate
  that the server is shut down.

### HttpExport

* **`http.enabled`** (`bool`) -
  Enables publishing metrics via HTTP POST to the metricz-exporter service.
* **`http.serialized`** (`bool`) = true -
  Serializes metrics into a JSON array `["metric 1", "metric 2"]` before
  sending. true - Uses engine native C++ serialization. Extremely fast
  (~3-5ms). Requires a backend that supports JSON arrays (e.g.
  metricz-exporter). false - Uses script-based string concatenation. Slower
  (~9ms buffered, ~30ms unbuffered). Recommended: true (approx. 2x faster
  than buffered text and 5x faster than unbuffered).
* **`http.buffer`** (`int`) = -1 -
  Buffer size (in lines) per HTTP POST request. <= 0 - Disable buffer, send
  all metrics in one request. > 0 - Send metrics chunked by the set line
  count. With 'serialized=true', the buffer setting has minimal impact on
  CPU performance. A value of -1 is recommended to reduce the number of HTTP
  requests. Use chunking (e.g. 512) only if you encounter network payload
  limits. With 'serialized=false' recommended range 64-512 for optimal
  performance. HTTP POST is not disk-dependent, but building one huge
  request body requires more CPU time.
* **`http.url`** (`string`) = "<http://127.0.0.1:8098>" -
  Remote URL of the metricz-exporter instance.
* **`http.user`** (`string`) = "metricz" -
  Username for Basic Auth protected publishing in metricz-exporter.
* **`http.password`** (`string`) -
  Password for Basic Auth protected publishing in metricz-exporter.
* **`http.max_retries`** (`int`) = 2 -
  Maximum number of retries if the request fails.
* **`http.retry_delay_ms`** (`int`) = 500 -
  Base delay (in milliseconds) between retry attempts. The delay will
  increase with each subsequent attempt and is also randomized within the
  range of 0.75x and 1.25x.
* **`http.retry_max_backoff_ms`** (`int`) = 5000 -
  Maximum allowed calculated delay (in milliseconds) for retries.
* **`http.read_timeout_sec`** (`int`) = 5 -
  Timeout in seconds for read operations.
* **`http.connect_timeout_sec`** (`int`) = 5 -
  Timeout in seconds for connection operations.

### DisabledMetrics

* **`disabled_metrics.rpc_input`** (`bool`) -
  Disables RPC metrics collection. `dayz_metricz_rpc_input_total`
* **`disabled_metrics.events`** (`bool`) -
  Disables event handler metrics collection. `dayz_metricz_events_total`
* **`disabled_metrics.http`** (`bool`) -
  Disables http metrics collection. This will not apply if remote mertic
  export is disabled. `dayz_metricz_http_*`
* **`disabled_metrics.players`** (`bool`) -
  Disables player-related metrics collection. `dayz_metricz_player_*`
* **`disabled_metrics.zombies`** (`bool`) -
  Disables zombie per-type and mind state metrics collection.
  `dayz_metricz_animals_by_type` and `dayz_metricz_infected_mind_state`
* **`disabled_metrics.animals`** (`bool`) -
  Disables animal per-type metrics collection.
  `dayz_metricz_infected_by_type`
* **`disabled_metrics.transports`** (`bool`) -
  Disables vehicle and transport metrics collection.
  `dayz_metricz_transport_*`
* **`disabled_metrics.weapons`** (`bool`) -
  Disables weapon per-type metrics collection (count, shots, kills, and
  hits). `dayz_metricz_weapon_shots_total`,
  `dayz_metricz_weapons_by_type_total`,
  `dayz_metricz_player_killed_by_total` and
  `dayz_metricz_creature_killed_by_total`
* **`disabled_metrics.hits`** (`bool`) -
  Disables metrics for players and zombies/animals hit by specific ammo
  types. `dayz_metricz_player_killed_by_total` and
  `dayz_metricz_creature_killed_by_total`
* **`disabled_metrics.kills`** (`bool`) -
  Disables metrics for players and zombies/animals killed by specific
  objects.
* **`disabled_metrics.territories`** (`bool`) -
  Disables territory flag metrics collection.
* **`disabled_metrics.areas`** (`bool`) -
  Disables EffectArea (Contaminated, Geyser, HotSpring, Volcanic, etc.)
  metrics.
* **`disabled_metrics.local_areas`** (`bool`) = true -
  Disables Local EffectArea metrics (e.g., ContaminatedArea_Local created
  from Grenade_ChemGas). This is disabled by default because metrics for
  such local zones will always have unique positions, thereby creating new
  metric series in the TSDB each time. Use with caution, as this may bloat
  your metrics database!
* **`disabled_metrics.positions`** (`bool`) -
  Disables player and transport coordinate metrics.
* **`disabled_metrics.positions_height`** (`bool`) = true -
  Disables player and transport height coordinate (Y) metrics. If
  `positions` is disabled, this will also be disabled forcibly.
* **`disabled_metrics.positions_yaw`** (`bool`) = true -
  Disables player and transport orientation metrics. If `positions` is
  disabled, this will also be disabled forcibly.

### Thresholds

* **`thresholds.hit_damage`** (`float`) = 3 -
  Minimum damage required to collect hit metrics in `EEHitBy()`. Values of
  -1 or less disable this threshold.
* **`thresholds.hit_damage_vehicle`** (`float`) = 15 -
  Minimum damage from vehicles required to collect hit metrics in
  `EEHitBy()`. Values of -1 or less disable this threshold.

### Geo

* **`geo.disable_world_coordinates`** (`bool`) -
  Disables conversion of coordinate metrics to the `EPSG:4326` (WGS84)
  format. By default, positions are converted to Longitude/Latitude in the
  -180/180 and -90/90 range. If disabled, all exported coordinates retain
  vanilla zero-relative meters.
* **`geo.world_effective_size`** (`float`) -
  Overrides the effective map tile size in world units. Useful if the web
  map size is larger than the game world size. (For example, iZurvive tiles
  for Chernarus have a size of `15926`, although the world size is `15360`).
