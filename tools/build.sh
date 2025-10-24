#!/usr/bin/env bash
set -eu

cd /p/metricz

./tools/validate.sh "$PWD"
./tools/config.sh "$PWD"
./tools/metrics.sh "$PWD"
./tools/astyle.sh "$PWD"
./tools/ctags.sh "$PWD"

pbo='/d/SteamLibrary/steamapps/common/DayZ Tools/Bin/AddonBuilder/AddonBuilder.exe'
src='P:\metricz'
mod='P:\@metricz\addons'
include="$src"'\tools\pbo_include.txt'

"$pbo" "$src" "$mod" -clear -include="$include" -prefix="metricz"
cp README.md /p/\@metricz/
