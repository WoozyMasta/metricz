# MetricZ Configuration

> Automatically generated list of configuration options from the source code.

This document lists configuration options and CLI flags
used by the **MetricZ** mod for DayZ server.

Each entry below shows:

1. the **serverDZ.cfg** option (with `MetricZ_` prefix)
2. the equivalent **CLI flag** (with `-metricz-` prefix)
3. a short description taken from the source code comments.

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
  `-metricz-init-delay` —
  Delay before the first metric collection in seconds
  (default: `60`);
* **`MetricZ_ScrapeInterval`**
  `-metricz-scrape-interval` —
  Interval between metric updates in seconds
  (default: `15`);
* **`MetricZ_DisablePlayerMetrics`**
  `-metricz-disable-player` —
  Disable player-related metrics collection;
* **`MetricZ_DisableZombieMetrics`**
  `-metricz-disable-zombie` —
  Disable zombie per-type and mind states metrics collection;
* **`MetricZ_DisableAnimalMetrics`**
  `-metricz-disable-animal` —
  Disable animal per-type metrics collection;
* **`MetricZ_DisableTransportMetrics`**
  `-metricz-disable-transport` —
  Disable vehicle and transport metrics collection;
* **`MetricZ_DisableWeaponMetrics`**
  `-metricz-disable-weapon` —
  Disable weapon usage metrics collection;
* **`MetricZ_DisableTerritoryMetrics`**
  `-metricz-disable-territory` —
  Disable territory flag metrics collection;
* **`MetricZ_DisableCoordinatesMetrics`**
  `-metricz-disable-coordinates` —
  Disable player and transport coordinate metrics;
* **`MetricZ_DisableGeoCoordinatesFormat`**
  `-metricz-disable-geo-coordinates-format` —
  Disable conversion of coordinates metrics to geo `EPSG:4326` (WGS84)
  format. By default convert position to lon/lat in `-180/180` and `-90/90`
  range. If disable, all exported coordinates hold vanilla zero relative
  meters;
* **`MetricZ_DisableRPCMetrics`**
  `-metricz-disable-rpc` —
  Disable RPC metrics collection;
* **`MetricZ_DisableEventMetrics`**
  `-metricz-disable-event` —
  Disable event handler metrics collection;
* **`MetricZ_MapEffectiveSize`**
  `-metricz-map-effective-size` —
  Override effective map tiles size in world units. Useful if the web map
  size is larger than the game world size (for example, the izurvive tiles
  for Chernarus have a size of `15926`, although the world size is `15360`)
  (default: `0`);
* `-metricz-map-tiles-name` —
  Override map tiles name. Useful if the name of the web map tiles was not
  recognized correctly;
* `-metricz-map-tiles-version` —
  Override map tiles version. Useful if the web map version has been updated
  but the MetricZ returns the old version;
* `-metricz-map-tiles-format` —
  Override map tiles format (e.g. `webp`, `jpg`, `png`);
