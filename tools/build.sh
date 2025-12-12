#!/usr/bin/env bash
set -eu

url() {
  printf '[InternetShortcut]\nURL=%s\n' "$1" >"/p/\@metricz/$2.url"
}

cd /p/metricz

./tools/validate.sh "$PWD"
./tools/update-contstants.sh "$PWD" true
./tools/config.sh "$PWD"
./tools/metrics.sh "$PWD"
./tools/astyle.sh "$PWD"
./tools/ctags.sh "$PWD"

pbo='/d/SteamLibrary/steamapps/common/DayZ Tools/Bin/AddonBuilder/AddonBuilder.exe'
src='P:\metricz'
mod='P:\@metricz\addons'
include="$src"'\tools\pbo_include.txt'

"$pbo" "$src" "$mod" -clear -include="$include" -prefix="metricz"
cp README.md CONFIG.md METRICS.md /p/\@metricz/

url 'https://steamcommunity.com/sharedfiles/filedetails/?id=3594119002' workshop
url 'https://github.com/WoozyMasta/metricz' github

./tools/update-contstants.sh "$PWD" false
