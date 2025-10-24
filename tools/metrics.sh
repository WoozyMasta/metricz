#!/usr/bin/env bash
set -euo pipefail

: "${PROJECT_DIR:=${1:-$PWD}}"
cd "$PROJECT_DIR"

out="METRICS.md"

cat >"$out" <<'EOF'
# MetricZ Scraped Metrics Reference

> Automatically generated list of metrics from the source code.

This document lists all metrics exposed by the **MetricZ** mod for
DayZ server. Each metric includes its identifier, type
(`GAUGE` or `COUNTER`), and description as defined in the source code.
EOF

while read -r file; do
  {
    printf '\n## [%s](%s)\n\n' "${file##*MetricZ/}" "$file"
    awk -f tools/metrics_extract.awk "$file"
  } >>"$out"
done < <(
  grep -RlE 'new\s+MetricZ_Metric(Int|Float)' ./scripts --include='*.c'
)
