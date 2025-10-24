#!/usr/bin/env bash
set -eu

: "${PROJECT_DIR:=${1:-$PWD}}"

cd "$PROJECT_DIR"
export LC_ALL=C

while read -r file; do
  first_line="$(tools/first_code_line.awk "$file")"
  if [ "${first_line:-}" != "#ifdef SERVER" ]; then
    echo "FAIL: not define SERVER: $file"
    exit 1
  fi

  if ! grep -q SPDX-License-Identifier "$file"; then
    echo "FAIL: not define SPDX: $file"
    exit 1
  fi

  if grep -n $'[\200-\377]' "$file"; then
    echo "FAIL: non-ASCII chars present: $file"
    exit 1
  fi
done < <(find ./scripts -type f -name "*.c")
