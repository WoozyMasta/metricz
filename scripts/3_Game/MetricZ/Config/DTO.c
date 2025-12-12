/*
    SPDX-License-Identifier: GPL-3.0-or-later
    Copyright (c) 2025 WoozyMasta
    Source: https://github.com/woozymasta/metricz
*/

#ifdef SERVER
/**
    \brief MetricZ Config DTO
*/
class MetricZ_ConfigDTO
{
	void MetricZ_ConfigDTO()
	{
		settings = new MetricZ_ConfigDTO_BaseSettings();
		file = new MetricZ_ConfigDTO_FileExport();
		http = new MetricZ_ConfigDTO_HttpExport();
		disabled_metrics = new MetricZ_ConfigDTO_DisabledMetrics();
		thresholds = new MetricZ_ConfigDTO_Thresholds();
		geo = new MetricZ_ConfigDTO_Geo();
	}

	// Internal configuration version. **Do not modify**.
	string version = MetricZ_Constants.VERSION;

	// Base settings for metric collection.
	ref MetricZ_ConfigDTO_BaseSettings settings;

	// Settings for exporting metrics to a local file.
	ref MetricZ_ConfigDTO_FileExport file;

	// Settings for publishing metrics via HTTP.
	ref MetricZ_ConfigDTO_HttpExport http;

	// Switches to disable specific metric series.
	ref MetricZ_ConfigDTO_DisabledMetrics disabled_metrics;

	// Metric collection thresholds.
	ref MetricZ_ConfigDTO_Thresholds thresholds;

	// Geographic coordinate settings.
	ref MetricZ_ConfigDTO_Geo geo;

	[NonSerialized()]
	int max_players = 255; // from serverDZ.cfg:maxPlayers

	[NonSerialized()]
	int fps_limit = 0; // from CLI:-limitFPS

	/**
	    \brief Normalizes configuration values within valid ranges.
	*/
	void Normalize()
	{
		settings.Normalize();
		file.Normalize();
		http.Normalize();
		disabled_metrics.Normalize();
		thresholds.Normalize();
		geo.Normalize();

		max_players = MetricZ_Helpers.GetLimitPlayers();
		fps_limit = MetricZ_Helpers.GetLimitFPS();
	}
}

class MetricZ_ConfigDTO_BaseSettings
{
	// Overrides the Instance ID. By default, this is detected automatically from serverDZ.cfg instanceID,
	// or uses gamePort/steamQueryPort if instanceID is undefined or zero.
	// This ID must be unique across all game servers on all hosts.
	// You can override it here to prevent storage and profile path changes on existing servers.
	string instance_id;

	// Delay in seconds before the first metric collection begins.
	int init_delay_sec = 30;

	// Interval in seconds between metric updates.
	int collect_interval_sec = 15;

	/**
	    \brief Normalizes configuration values within valid ranges.
	*/
	void Normalize()
	{
		instance_id = MetricZ_Helpers.GetInstanceID(instance_id);
		init_delay_sec = Math.Clamp(init_delay_sec, 0, 300);
		collect_interval_sec = Math.Clamp(collect_interval_sec, 0, 900);
	}
}

class MetricZ_ConfigDTO_FileExport
{
	// Enables saving metrics to a local file (`metricz.prom`) for collection
	// by node-exporter or windows-exporter textfile collector.
	bool enabled = true;

	// Buffer size (in lines) for file writing.
	// 0 - Disable buffer, write each line to the file separately.
	// Positive value - Write to file buffered by line count (recommended).
	// Negative value - Use unlimited buffer size, write the whole buffer at the end (not recommended).
	//
	// Recommended range: 16-64 for optimal performance.
	// This ensures low I/O but maintains a reasonable buffer.
	// Values of 2048 or larger cause huge memory allocations and can drop performance.
	int buffer = 32;

	// Enables atomic export. Metrics are written to a temporary file and copied/renamed
	// to the .prom file upon completion, removing the temp file.
	// This adds extra I/O overhead and takes nearly x2-3 as long to write,
	// but it is crucial to prevent the collector from reading incomplete files.
	// Disabling this is not recommended.
	bool atomic = true;

	// File name override (without extension)
	// Default file path `$profile:metricz/export/metricz_${instance_id}.prom`.
	// File path with override `$profile:metricz/export/${file_name}.prom`.
	string file_name;

	// Delete the PROM file when shutting down the server.
	// This is not recommended to be enabled by default,
	// since upon completion of work, a single metric `dayz_metricz_status=0` is
	// written to the file to indicate that the server is shut down.
	bool delete_on_shutdown;

	[NonSerialized()]
	string prom_file_path;
	[NonSerialized()]
	string temp_file_path;

	/**
	    \brief Normalizes configuration values within valid ranges.
	*/
	void Normalize()
	{
		buffer = Math.Clamp(buffer, -1, MetricZ_Constants.MAX_BUFFER_SIZE);
	}
}

class MetricZ_ConfigDTO_HttpExport
{
	// Enables publishing metrics via HTTP POST to the metricz-exporter service.
	bool enabled;

	// Buffer size (in lines) per HTTP POST request.
	// <= 0 - Disable buffer, send all metrics in one request.
	// > 0 - Send metrics chunked by the set line count.
	//
	// Recommended range: 64-512 for optimal performance.
	// HTTP POST is not disk-dependent, but building one huge request body requires more CPU time.
	// If you experience high latency or network issues, try disabling the buffer.
	int buffer = 128;

	// Remote URL of the metricz-exporter instance.
	string url = "http://127.0.0.1:8098";

	// Username for Basic Auth protected publishing in metricz-exporter.
	string user = "metricz";

	// Password for Basic Auth protected publishing in metricz-exporter.
	string password;

	// Maximum number of retries if the request fails.
	int max_retries = 2;

	// Base delay (in milliseconds) between retry attempts.
	// The delay will increase with each subsequent attempt and is also
	// randomized within the range of 0.75x and 1.25x.
	int retry_delay_ms = 500;

	// Maximum allowed calculated delay (in milliseconds) for retries.
	int retry_max_backoff_ms = 5000;

	// Timeout in seconds for read operations.
	int read_timeout_sec = 5;

	// Timeout in seconds for connection operations.
	int connect_timeout_sec = 5;

	/**
	    \brief Normalizes configuration values within valid ranges.
	*/
	void Normalize()
	{
		buffer = Math.Clamp(buffer, -1, MetricZ_Constants.MAX_BUFFER_SIZE);
		if (buffer == 0)
			buffer = -1;

		url = MetricZ_Helpers.GetActiveURL(url, user, password);
		max_retries = Math.Clamp(max_retries, 0, 10);
		retry_delay_ms = Math.Clamp(retry_delay_ms, 100, MetricZ_Config.Get().settings.init_delay_sec * 1000);
		retry_max_backoff_ms = Math.Clamp(retry_max_backoff_ms, 1000, MetricZ_Config.Get().settings.collect_interval_sec * 1000);
		read_timeout_sec = Math.Clamp(read_timeout_sec, 3, 120);
		connect_timeout_sec = Math.Clamp(connect_timeout_sec, 3, 120);

		if (url == string.Empty) {
			enabled = false;
			ErrorEx("MetricZ: rest-url is empty", ErrorExSeverity.WARNING);
		}
	}
}

class MetricZ_ConfigDTO_DisabledMetrics
{
	// Disables RPC metrics collection.
	// `dayz_metricz_rpc_input_total`
	bool rpc_input;

	// Disables event handler metrics collection.
	// `dayz_metricz_events_total`
	bool events;

	// Disables http metrics collection.
	// This will not apply if remote mertic export is disabled.
	// `dayz_metricz_http_*`
	bool http;

	// Disables player-related metrics collection.
	// `dayz_metricz_player_*`
	bool players;

	// Disables zombie per-type and mind state metrics collection.
	// `dayz_metricz_animals_by_type` and `dayz_metricz_infected_mind_state`
	bool zombies;

	// Disables animal per-type metrics collection.
	// `dayz_metricz_infected_by_type`
	bool animals;

	// Disables vehicle and transport metrics collection.
	// `dayz_metricz_transport_*`
	bool transports;

	// Disables weapon per-type metrics collection (count, shots, kills, and hits).
	// `dayz_metricz_weapon_shots_total`, `dayz_metricz_weapons_by_type_total`,
	// `dayz_metricz_player_killed_by_total` and `dayz_metricz_creature_killed_by_total`
	bool weapons;

	// Disables metrics for players and zombies/animals hit by specific ammo types.
	// `dayz_metricz_player_killed_by_total` and `dayz_metricz_creature_killed_by_total`
	bool hits;

	// Disables metrics for players and zombies/animals killed by specific objects.
	bool kills;

	// Disables territory flag metrics collection.
	bool territories;

	// Disables EffectArea (Contaminated, Geyser, HotSpring, Volcanic, etc.) metrics.
	bool areas;

	// Disables Local EffectArea metrics (e.g., ContaminatedArea_Local created from Grenade_ChemGas).
	// This is disabled by default because metrics for such local zones will always have unique
	// positions, thereby creating new metric series in the TSDB each time.
	// Use with caution, as this may bloat your metrics database!
	bool local_areas = true;

	// Disables player and transport coordinate metrics.
	bool positions;

	// Disables player and transport height coordinate (Y) metrics.
	// If `positions` is disabled, this will also be disabled forcibly.
	bool positions_height = true;

	// Disables player and transport orientation metrics.
	// If `positions` is disabled, this will also be disabled forcibly.
	bool positions_yaw = true;

	/**
	    \brief Normalizes configuration values within valid ranges.
	*/
	void Normalize() {}
}

class MetricZ_ConfigDTO_Thresholds
{
	// Minimum damage required to collect hit metrics in `EEHitBy()`.
	// Values of -1 or less disable this threshold.
	float hit_damage = 3;

	// Minimum damage from vehicles required to collect hit metrics in `EEHitBy()`.
	// Values of -1 or less disable this threshold.
	float hit_damage_vehicle = 15;

	/**
	    \brief Normalizes configuration values within valid ranges.
	*/
	void Normalize()
	{
		hit_damage = Math.Clamp(hit_damage, -1, 100);
		hit_damage_vehicle = Math.Clamp(hit_damage_vehicle, -15, 100);
	}
}

class MetricZ_ConfigDTO_Geo
{
	// Disables conversion of coordinate metrics to the `EPSG:4326` (WGS84) format.
	// By default, positions are converted to Longitude/Latitude in the -180/180 and -90/90 range.
	// If disabled, all exported coordinates retain vanilla zero-relative meters.
	bool disable_world_coordinates;

	// Overrides the effective map tile size in world units.
	// Useful if the web map size is larger than the game world size.
	// (For example, iZurvive tiles for Chernarus have a size of `15926`, although the world size is `15360`).
	float world_effective_size;

	/**
	    \brief Normalizes configuration values within valid ranges.
	*/
	void Normalize()
	{
		world_effective_size = Math.Clamp(world_effective_size, g_Game.GetWorld().GetWorldSize(), 81920);
	}
}
#endif
