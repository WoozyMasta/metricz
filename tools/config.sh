#!/usr/bin/env bash
set -euo pipefail

: "${PROJECT_DIR:=${1:-$PWD}}"
cd "$PROJECT_DIR"

out="CONFIG.md"
config_file="./scripts/3_Game/MetricZ/Config/DTO.c"

{
  cat ./tools/config.md
  printf '\n## Options [%s](%s)\n' "${config_file##*MetricZ/}" "$config_file"
  awk -f tools/config_extract.awk "$config_file"
} >"$out"
