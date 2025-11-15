#!/usr/bin/awk -f

BEGIN {
  RS = ";"
}

function wrap(str, width, out, n, w, i) {
  n = split(str, a, / /)
  out = a[1]; w = length(out)
  for (i = 2; i <= n; i++) {
    if (w + 1 + length(a[i]) > width) {
      out = out "\n  " a[i]
      w = length(a[i])
    } else {
      out = out " " a[i]
      w += 1 + length(a[i])
    }
  }
  return out
}

{
  re = "new MetricZ_Metric(Int|Float)[[:space:]]*\\(" \
       "[[:space:]]*\"([^\"]+)\"" \
       "(" \
         "[[:space:]]*,[[:space:]]*\"([^\"]*)\"" \
         "(" \
           "[[:space:]]*,[[:space:]]*MetricZ_MetricType\\.([A-Z]+)" \
         ")?" \
       ")?"

  if (match($0, re, m)) {
    name = m[2]

    # type: default GAUGE if not specified
    if (m[6] != "")
      type = m[6]
    else
      type = "GAUGE"

    if (type == "COUNTER")
      name = name "_total"

    if (m[4] != "") {
      desc = m[4]
    } else {
      desc = name
      gsub("_", " ", desc)
    }

    desc = wrap(desc, 76)
    printf "* **`dayz_metricz_%s`** (`%s`) —\n  %s\n", name, type, desc
  }
}
