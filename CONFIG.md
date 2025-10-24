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

## Options [Utils/Config.c](./scripts/3_Game/MetricZ/Utils/Config.c)

* **`MetricZ_InitDelay`** (`-metricz-init-delay`) —
  Delay before the first metric collection in seconds (default 60)
* **`MetricZ_ScrapeInterval`** (`-metricz-scrape-interval`) —
  Interval between metric updates in seconds (default 15, min 1)
* **`MetricZ_DisablePlayerMetrics`** (`-metricz-disable-player`) —
  Disable player-related metrics collection
* **`MetricZ_DisableTransportMetrics`** (`-metricz-disable-transport`) —
  Disable vehicle and transport metrics collection
* **`MetricZ_DisableWeaponMetrics`** (`-metricz-disable-weapon`) —
  Disable weapon usage metrics collection
* **`MetricZ_DisableTerritoryMetrics`** (`-metricz-disable-territory`) —
  Disable territory flag metrics collection
* **`MetricZ_EnableCoordinatesMetrics`** (`-metricz-enable-coordinates`) —
  Enable player coordinate metrics (off by default)
* **`MetricZ_DisableRPCMetrics`** (`-metricz-disable-rpc`) —
  Disable RPC metrics collection
* **`MetricZ_DisableEventMetrics`** (`-metricz-disable-event`) —
  Disable event handler metrics collection
