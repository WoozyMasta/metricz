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
