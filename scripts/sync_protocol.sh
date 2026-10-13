#!/usr/bin/env bash
# Copies common/protocol.h into both board projects.
# Arduino IDE can't include files outside the sketch folder, so each side keeps a copy.
# Run this after every change to common/protocol.h and commit the copies too.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/common/protocol.h"

cp "$SRC" "$ROOT/esp32/as4_esp32/protocol.h"
echo "-> esp32/as4_esp32/protocol.h"

# MCXC444: copy into the MCUXpresso project's source/ folder once it exists
for d in "$ROOT"/mcxc444/*/source; do
  [ -d "$d" ] || continue
  cp "$SRC" "$d/protocol.h"
  echo "-> ${d#$ROOT/}/protocol.h"
done
cp "$SRC" "$ROOT/mcxc444/skeleton/protocol.h"
echo "-> mcxc444/skeleton/protocol.h"
