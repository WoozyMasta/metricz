#!/usr/bin/awk -f

function wrap(str, width, out, n, w) {
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
  re = "MetricZ_Metric(Int|Float)[[:space:]]*\\(" \
       "[[:space:]]*\"([^\"]+)\"[[:space:]]*," \
       "[[:space:]]*\"([^\"]+)\"[[:space:]]*," \
       "[[:space:]]*MetricZ_MetricType\\.([A-Z]+)"

  if (match($0, re, m)) {
    name = m[2]
    type = m[4]
    if (type == "COUNTER") name = name "_total"
    desc = wrap(m[3], 76)
    printf "* **`dayz_metricz_%s`** (`%s`) —\n  %s\n", name, type, desc
  }
}
