#!/usr/bin/env bash
set -euo pipefail
repo="$(cd "$(dirname "$0")/.." && pwd)"
export PATH="/opt/homebrew/bin:/usr/bin:/bin:/usr/sbin:/sbin:$PATH"
idf_root="${AI_PASSPORT_IDF_ROOT:-$HOME/esp/esp-idf-v5.5.3}"
source "$idf_root/export.sh"
cd "$repo"
./tools/validate.sh
