#!/usr/bin/awk -f

function print_wrapped(text, words, i, w, line, len, n) {
  gsub(/[ \t]+/, " ", text)
  gsub(/^ | $/, "", text)

  if (text == "") return

  n = split(text, words, " ")
  line = "  "
  len = 2

  for (i = 1; i <= n; i++) {
    w = words[i]
    if (len + length(w) + 1 > 76) {
      print line
      line = "  " w
      len = 2 + length(w)
    } else {
      if (line == "  ") {
        line = line w
        len += length(w)
      } else {
        line = line " " w
        len += 1 + length(w)
      }
    }
  }
  if (line != "  ") print line
}

BEGIN {
  in_comment_block = 0
  desc_buffer = ""
  skip_next = 0
  current_prefix = ""
}

{
  raw_line = $0
  gsub(/^[ \t]+|[ \t]+$/, "", raw_line)
}

/^[ \t]*#/ {
  desc_buffer = ""
  next
}

/\/\*/ {
  in_comment_block = 1
  sub(/^[ \t]*\/\*[ \t]*/, "", raw_line)
  if (raw_line !~ /\*\//) {
    if (raw_line != "") desc_buffer = desc_buffer " " raw_line
    next
  }
}

/\*\// {
  in_comment_block = 0
  sub(/[ \t]*\*\/.*$/, "", raw_line)
  if (raw_line != "") desc_buffer = desc_buffer " " raw_line
  next
}

in_comment_block {
  sub(/^\*[ \t]*/, "", raw_line)
  sub(/\\brief[ \t]*/, "", raw_line)
  if (raw_line != "") desc_buffer = desc_buffer " " raw_line
  next
}

/^[ \t]*\/\// {
  sub(/^[ \t]*\/\/[ \t]*/, "", raw_line)
  if (raw_line !~ /^---/) {
    desc_buffer = desc_buffer " " raw_line
  }
  next
}

/\[NonSerialized/ {
  skip_next = 1
  next
}

/^class / {
  match($0, /class [a-zA-Z0-9_]+/)
  class_name = substr($0, RSTART+6, RLENGTH-6)

  if (class_name in class_map) {
    current_prefix = class_map[class_name] "."
  } else {
    current_prefix = ""
  }

  if (class_name == "MetricZ_ConfigDTO") {
    display_name = "Config"
  } else {
    sub(/^MetricZ_ConfigDTO_/, "", class_name)
    display_name = class_name
  }

  print ""
  print "### " display_name
  print ""

  desc_buffer = ""
  skip_next = 0
  next
}

/void [a-zA-Z0-9_]+\(/ || (/^[a-zA-Z0-9_]+ \(/) {
  desc_buffer = ""
  skip_next = 0
  next
}

/;/ && !/\(/ {
  if ($0 ~ /^[ \t]*(ref )?[a-zA-Z0-9_<>]+[ \t]+[a-zA-Z0-9_]+/) {
    if (skip_next) {
      skip_next = 0
      desc_buffer = ""
      next
    }

    line = raw_line
    eq_idx = index(line, "=")
    semi_idx = index(line, ";")
    end_decl_idx = (eq_idx > 0) ? eq_idx : semi_idx

    declaration_part = substr(line, 1, end_decl_idx - 1)
    sub(/[ \t]+$/, "", declaration_part)

    n = split(declaration_part, parts, " ")
    name = parts[n]
    type = parts[1]
    for (i=2; i<n; i++) type = type " " parts[i]

    clean_type = type
    sub(/^ref /, "", clean_type)
    class_map[clean_type] = name

    default_val = ""
    if (eq_idx > 0) {
      val_part = substr(line, eq_idx + 1, semi_idx - eq_idx - 1)
      gsub(/^[ \t]+|[ \t]+$/, "", val_part)
      if (substr(val_part, 1, 1) == "\"") {
        match(val_part, /".*"/)
        val_part = substr(val_part, RSTART, RLENGTH)
      } else {
        sub(/[ \t]*\/\/.*$/, "", val_part)
        gsub(/[ \t]+$/, "", val_part)
      }
      default_val = val_part
    }

    header = "* **`" current_prefix name "`** (`" type "`)"
    if (default_val != "") {
      header = header " = " default_val
    }
    header = header " -"

    print header
    print_wrapped(desc_buffer)

    desc_buffer = ""
    next
  }
}

/}/ {
  desc_buffer = ""
  skip_next = 0
}
