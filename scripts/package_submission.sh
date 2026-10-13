#!/usr/bin/env bash
# Builds the code zip for Canvas.
#   Interim: ./scripts/package_submission.sh interim B01 10   -> B01_10_Interim_Code.zip
#   Final:   ./scripts/package_submission.sh final   B01 10   -> B01_10_Code.zip
# Excludes secrets.h, build output and git metadata.
set -euo pipefail
STAGE="${1:?interim|final}"; SLOT="${2:?lab slot e.g. B01}"; GROUP="${3:?group no. e.g. 10}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
case "$STAGE" in
  interim) OUT="${SLOT}_${GROUP}_Interim_Code.zip" ;;
  final)   OUT="${SLOT}_${GROUP}_Code.zip" ;;
  *) echo "stage must be interim or final"; exit 1 ;;
esac
cd "$ROOT"
rm -f "$OUT"
zip -r "$OUT" common esp32 mcxc444 \
  -x '*/secrets.h' '*/Debug/*' '*/Release/*' '*/.metadata/*' '*.o' '*.d' '*.axf' '*.bin' '*.map'
echo "Created $OUT — upload it to Canvas (don't put the report PDF inside the zip)."
