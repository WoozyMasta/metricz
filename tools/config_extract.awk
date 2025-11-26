#!/usr/bin/awk -f

function trim(s) {
  sub(/^[[:space:]]+/, "", s)
  sub(/[[:space:]]+$/, "", s)
  return s
}

# simple word-wrap to given width with indent
# возвращает строку, сам ничего не печатает
function wrap(text, width, indent,   n, words, i, line, base_len, w, out) {
  text = trim(text)
  if (text == "")
    return ""

  n = split(text, words, /[[:space:]]+/)
  line = indent
  base_len = length(indent)

  for (i = 1; i <= n; i++) {
    w = words[i]
    if (w == "")
      continue

    # length if we append this word (with leading space if not first)
    if (line == indent)
      new_len = base_len + length(w)
    else
      new_len = length(line) + 1 + length(w)

    if (new_len > width) {
      # flush current line and start new one
      if (line != indent)
        out = (out ? out "\n" : "") line

      line = indent w
    } else {
      if (line == indent)
        line = indent w
      else
        line = line " " w
    }
  }

  if (line != indent)
    out = (out ? out "\n" : "") line

  return out
}

{
  # accumulate comment lines starting with //
  if ($0 ~ /^[[:space:]]*\/\//) {
    c = substr($0, index($0, "//") + 2)
    comment = (comment ? comment " " : "") trim(c)
    next
  }

  re = "^[[:space:]]*.*(GetNumber|GetString|GetBool)[[:space:]]*\\(" \
       "[[:space:]]*\"([^\"]+)\"[[:space:]]*" \
       "(,[[:space:]]*\"([^\"]+)\")?" \
       "([[:space:]]*,[[:space:]]*([^,)]*))?"

  if (match($0, re, m)) {
    type = m[1]
    arg1 = m[2]
    arg2 = m[4]

    def = ""
    if (m[6] != "")
      def = trim(m[6])

    desc = comment ? comment : "(no description)"
    comment = ""

    if (arg2 != "")
      printf "* **`MetricZ_%s`**\n  `-metricz-%s` —\n", arg1, arg2
    else
      printf "* `-metricz-%s` —\n", arg1

    wrapped = wrap(desc, 76, "  ")
    if (wrapped != "")
      printf "%s", wrapped   # печать без завершающего \n

    if (def != "")
      printf "\n  (default: `%s`)", def

    printf ";\n"

    next
  }

  comment = ""
}
