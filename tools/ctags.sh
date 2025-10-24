#!/usr/bin/env bash
set -eu

: "${PROJECT_DIR:=${1:-$PWD}}"

cd "$PROJECT_DIR"
ctags --recurse --exclude=.git --tag-relative --totals=yes \
  --languages='C#' --langmap='C#:+.c' --extras'=+fq' --fields='+nKS' \
  -f "$PROJECT_DIR/tags" .
