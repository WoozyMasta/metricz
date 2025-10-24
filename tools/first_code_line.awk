#!/usr/bin/awk -f

BEGIN { inblock=0 }
{
  line=$0
  sub(/^[ \t\r]+/, "", line)

  # strip /* ... */ blocks possibly spanning lines
  while (1) {
    if (inblock==1) {
      p=index(line,"*/")
      if (p==0) { next }
      line=substr(line,p+2)
      sub(/^[ \t\r]+/, "", line)
      inblock=0
      continue
    }
    if (line ~ /^\/\*/) {
      inblock=1
      line=substr(line,3)
      sub(/^[ \t\r]+/, "", line)
      continue
    }
    break
  }

  # skip empty and // comments
  if (line=="" || line ~ /^\/\//) next

  print line
  exit
}
