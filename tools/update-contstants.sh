#!/usr/bin/env bash
set -euo pipefail

: "${PROJECT_DIR:=${1:-$PWD}}"
: "${SET_DATA:=${2:-true}}"
cd "$PROJECT_DIR"

if [ "${SET_DATA,,}" == true ]; then
  version="$(git describe --tags --abbrev=0 2>/dev/null || echo "0.0.0")"
  commit="$(git rev-parse --short HEAD)"
  date="$(date -uIs)"
else
  version=dev
  commit=fffffff
  date=1970-01-01T00:00:00+00:00
fi

sed -i \
  -e "s/VERSION = \".*\"/VERSION = \"$version\"/" \
  -e "s/COMMIT_SHA = \".*\"/COMMIT_SHA = \"$commit\"/" \
  -e "s/BUILD_DATE = \".*\"/BUILD_DATE = \"$date\"/" \
  scripts/3_Game/MetricZ/Config/Constants.c
