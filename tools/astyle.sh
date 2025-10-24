#!/usr/bin/env bash
set -eu

: "${PROJECT_DIR:=${1:-$PWD}}"
: "${ASTYLE_RC:=${2:-.astylerc}}"

cd "$PROJECT_DIR"
astyle --project="$ASTYLE_RC" ./'*.c' ./'*.cpp'
