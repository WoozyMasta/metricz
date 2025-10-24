#!/usr/bin/env bash
set -euo pipefail

: "${PROJECT_DIR:=${1:-$PWD}}"
cd "$PROJECT_DIR"

out="CONFIG.md"
config_file="./scripts/3_Game/MetricZ/Utils/Config.c"

cat >"$out" <<'EOF'
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

EOF

{
  printf '## Options [%s](%s)\n\n' "${config_file##*MetricZ/}" "$config_file"
  awk -f tools/config_extract.awk "$config_file"
} >>"$out"
