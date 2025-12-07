#!/usr/bin/awk -f

function trim(s) {
  sub(/^[[:space:]]+/, "", s)
  sub(/[[:space:]]+$/, "", s)
  return s
}

# simple word-wrap to given width with indent
function wrap(text, width, indent,   n, words, i, line, base_len, w, out, new_len) {
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

    if (line == indent)
      new_len = base_len + length(w)
    else
      new_len = length(line) + 1 + length(w)

    if (new_len > width) {
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

BEGIN {
  in_public = 0
  comment = ""
}

{
  if ($0 ~ /\/\/[[:space:]]*\* <start json options>/) {
    in_public = 1
    comment = ""
    next
  }
  if ($0 ~ /\/\/[[:space:]]*\* <end json options>/) {
    in_public = 0
    comment = ""
    next
  }

  if (!in_public)
    next

  if ($0 ~ /^[[:space:]]*\/\//) {
    c = substr($0, index($0, "//") + 2)
    c = trim(c)
    if (c != "")
      comment = (comment ? comment " " : "") c
    next
  }

  if (match($0, /^[[:space:]]*(int|bool|float)[[:space:]]+([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*(=[^;]+)?;/, m)) {
    type = m[1]
    name = m[2]
    init = m[3]

    # default value
    def = ""
    if (init != "") {
      # init like "= 60" or "= true"
      def = init
      sub(/^=/, "", def)
      def = trim(def)
    } else {
      if (type == "bool")
        def = "false"
      else if (type == "int")
        def = "0"
      else if (type == "float")
        def = "0.0"
      else
        def = ""
    }

    desc = comment ? comment : "(no description)"
    comment = ""

    printf "* **`%s`** (`%s`) —\n", name, type

    wrapped = wrap(desc, 76, "  ")
    if (wrapped != "")
      printf "%s", wrapped

    if (def != "")
      printf "\n  (default: `%s`)", def

    printf "\n"

    next
  }

  if ($0 ~ /[^[:space:]]/)
    comment = ""
}
