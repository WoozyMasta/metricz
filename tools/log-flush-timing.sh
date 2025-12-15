#!/usr/bin/env bash

: "${LOG_FILE:=${1:?Usage: $0 <path_to_log_file>}}"

[ ! -f "$LOG_FILE" ] && {
  >&2 echo "File $LOG_FILE not found"
  exit 1
}

printf "%-20s | %-6s | %-9s | %-9s | %-9s\n" "METRIC" "COUNT" "MIN(ms)" "MAX(ms)" "AVG(ms)"
echo "------------------------------------------------------------------"

grep "Flush\." "$LOG_FILE" |
  awk '
{
    name = "unknown";
    val = 0;

    for(i=1; i<=NF; i++) {
      if($i ~ /^Flush\./) {
        name = $i;
        sub(/^Flush\./, "", name);
        break;
      }
    }

    if (name == "unknown") {
      if ($0 ~ /All Flush took/) name = "TOTAL_Flush";
      else if ($0 ~ /Sink.Begin took/) name = "Sink_Begin";
      else if ($0 ~ /Sink.End took/) name = "Sink_End";
    }

    if (name != "unknown") {
      val_str = $NF;
      sub(/ms$/, "", val_str);
      val = val_str + 0;

      sum[name] += val;
      count[name]++;

      if (count[name] == 1) {
        min[name] = val;
        max[name] = val;
      } else {
        if (val < min[name]) min[name] = val;
        if (val > max[name]) max[name] = val;
      }
    }
  }
  END {
    for (c in count) {
      printf "%-20s | %-6d | %-9.4f | %-9.4f | %-9.4f\n", c, count[c], min[c], max[c], sum[c]/count[c];
    }
  }' | sort -t '|' -k 5 -rn
