#!/usr/bin/env bash
set -euo pipefail

: "${URL:=https://dayz.xam.nu}"

declare -A ids=(
  ["alteria"]=3296994216
  ["anastara"]=2973953648
  ["antoria"]=2966495799
  ["arsteinen"]=2982575649
  ["avalon"]=2982575649
  ["azalea"]=3225634444
  ["banov"]=2415195639
  ["bitterroot"]=2906823750
  ["chernarusplus"]=221100
  ["chiemsee"]=1580589252
  ["deadfall"]=3050117454
  ["deerisle"]=1602372402
  ["esseker"]=2462896799
  ["hashima"]=2781560371
  ["iztek"]=2978912938
  ["livonia"]=221100 # Livonia / Enoch
  ["malvinas"]=3390173486
  ["melkart"]=2716445223
  ["namalsk"]=2289456201
  ["nasdara"]=3816030 # Badlands
  ["nhchernobyl"]=2727569951
  ["novikostok"]=3553075906
  ["nukezzone"]=2828800987
  ["nyheim"]=2633522605
  ["onforin"]=3445058656
  ["pnw"]=3290318225
  ["pripyat"]=3136720512
  ["raman"]=3401182744
  ["ros"]=2390417799
  ["rostow"]=2344585107
  ["sahrani"]=3508468461
  ["sakhal"]=2968040 # FrostLine
  ["stalkerdzone"]=2951293993
  ["stuartisland"]=1936423383
  ["swansisland"]=2517396668
  ["takistanplus"]=2563233742
  ["valning"]=1880753439
  ["vela"]=2794308565
  ["visisland"]=3031438368
  ["yiprit"]=2780320171
)

bundle="$(
  curl -sSfL "$URL/" |
    grep -oP 'src=["'\'']\K/js/bundle[^"'\'']+\.js' |
    head -n 1 | tr -d '\r'
)"

printf '\t// %s\n' "${bundle#*.}"

while IFS=$'\t' read -r json map; do
  [ "$map" == elderwoodz ] && continue
  [ "$map" == stalkerdzone ] && continue
  [ "${map: -3}" == exp ] && continue

  [ "$map" == livonia ] && printf '\tcase "enoch":\n'
  [ "$map" == banov ] && printf '\tcase "banovfrost":\n'

  printf '\tcase "%s":' "$map"

  curl -sSfL "https://static.xam.nu/dayz/json/$json" |
    jq -er '
      .info
      | " // size \(.size)\n" +
        "\t\ts_MapTilesVersion = \"\(.tilev)\";\n" +
        "\t\ts_MapTilesFormat = \"\(.format // "webp")\";"
    '

  [ -n "${ids[$map]-}" ] &&
    printf '\t\ts_MapWorkShopID = %s;\n' "${ids[$map]-}"

  [ "$map" == livonia ] && printf '\t\ts_MapTilesName = "livonia";\n'
  [ "$map" == banov ] && printf '\t\ts_MapTilesName = "banov";\n'

  printf '\t\tbreak;\n\n'

done < <(
  curl -sSfL "$URL/${bundle#/}" |
    grep -oP "JSON\.parse\('\K.*?(?='\))" |
    grep -m1 "chernarusplus" |
    jq -r '
      to_entries
      | sort_by(.key)
      | .[]
      | (if .value.i == 0 then "" else "-\(.value.i)" end) as $suffix
      | ["\(.key)/\(.value.v)\($suffix).json", .key]
      | @tsv
    ' |
    tr -d '\r'
)
