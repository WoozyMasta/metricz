#!/usr/bin/awk -f

function trim(s) {
  sub(/^[[:space:]]+/, "", s)
  sub(/[[:space:]]+$/, "", s)
  return s
}

{
  # accumulate comment lines
  if ($0 ~ /^[[:space:]]*\/\//) {
    c = substr($0, index($0, "//") + 2)
    comment = (comment ? comment " " : "") trim(c)
    next
  }

  # match Seconds or Toggle with two string args
  re = "^[[:space:]]*.*(Seconds|Toggle)[[:space:]]*\\(" \
       "[[:space:]]*\"([^\"]+)\"[[:space:]]*," \
       "[[:space:]]*\"([^\"]+)\""
  if (match($0, re, m)) {
    type = m[1]
    name = m[2]
    cli  = m[3]
    desc = comment ? comment : "(no description)"
    comment = ""
    printf "* **`MetricZ_%s`** (`-metricz-%s`) —\n  %s\n", name, cli, desc
    next
  }

  # reset if non-comment non-match
  comment = ""
}
