#!/usr/bin/env bash
set -euo pipefail

CONFIG="/home/user/Documents/DEV/BlockSci/blocksci-data/config.json"
PARSER_BIN="/home/user/Documents/DEV/BlockSci/build-dbg/tools/parser/blocksci_parser"
STEP=10000
LOOPS=1
DRY_RUN=0
MAX_BLOCKS=930000
STOP_REQUESTED=0
INT_COUNT=0
CURRENT_CHILD_PID=""
CURRENT_TAIL_PID=""
BITCOIN_MOUNT_PATH="/media/user/BTC1/bitcoin"

usage() {
  cat <<'EOF'
Usage: chunked_blocksci_update.sh [options]

Options:
  --config PATH        Path to BlockSci config.json
  --parser-bin PATH    Path to blocksci_parser binary
  --step N             Increment for parser.maxBlockNum per loop (default: 10000)
  --loops N            Number of increment+update cycles to run (default: 1)
  --max-blocks N       Hard cap for parser.maxBlockNum (default: 930000)
  --dry-run            Only update config, do not run parser commands
                      Ctrl+C once: stop after current loop
                      Ctrl+C twice: force stop immediately
  -h, --help           Show this help
EOF
}

on_int() {
  INT_COUNT=$((INT_COUNT + 1))

  if [[ "$INT_COUNT" -eq 1 ]]; then
    STOP_REQUESTED=1
    echo
    echo "SIGINT received: will stop after current loop finishes."
    return
  fi

  echo
  echo "Second SIGINT received: forcing immediate stop."
  if [[ -n "$CURRENT_TAIL_PID" ]] && kill -0 "$CURRENT_TAIL_PID" 2>/dev/null; then
    kill -TERM "$CURRENT_TAIL_PID" 2>/dev/null || true
  fi
  if [[ -n "$CURRENT_CHILD_PID" ]] && kill -0 "$CURRENT_CHILD_PID" 2>/dev/null; then
    kill -TERM "$CURRENT_CHILD_PID" 2>/dev/null || true
  fi
  exit 130
}

trap on_int INT

run_parser_step() {
  local label="$1"
  local log_path="$2"
  shift 2

  echo "$label (log: $log_path)"

  # Run parser detached from terminal signal group so Ctrl+C can be handled
  # by this wrapper without killing the parser mid-loop.
  setsid "$PARSER_BIN" "$CONFIG" "$@" >"$log_path" 2>&1 &
  CURRENT_CHILD_PID=$!

  tail -n +1 -f --pid="$CURRENT_CHILD_PID" "$log_path" &
  CURRENT_TAIL_PID=$!

  local status=0
  while true; do
    if wait "$CURRENT_CHILD_PID"; then
      status=0
      break
    fi
    status=$?
    if kill -0 "$CURRENT_CHILD_PID" 2>/dev/null; then
      continue
    fi
    break
  done

  if [[ -n "$CURRENT_TAIL_PID" ]]; then
    wait "$CURRENT_TAIL_PID" 2>/dev/null || true
  fi

  CURRENT_CHILD_PID=""
  CURRENT_TAIL_PID=""
  return "$status"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --config)
      CONFIG="$2"
      shift 2
      ;;
    --parser-bin)
      PARSER_BIN="$2"
      shift 2
      ;;
    --step)
      STEP="$2"
      shift 2
      ;;
    --loops)
      LOOPS="$2"
      shift 2
      ;;
    --max-blocks)
      MAX_BLOCKS="$2"
      shift 2
      ;;
    --dry-run)
      DRY_RUN=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage
      exit 1
      ;;
  esac
done

if [[ ! -f "$CONFIG" ]]; then
  echo "Config not found: $CONFIG" >&2
  exit 1
fi

if [[ ! -d "$BITCOIN_MOUNT_PATH" ]]; then
  echo "Mount path does not exist: $BITCOIN_MOUNT_PATH" >&2
  exit 1
fi

if ! mountpoint -q "$BITCOIN_MOUNT_PATH"; then
  # coinDirectory is often a subdirectory on the mounted volume (e.g. /media/.../bitcoin),
  # so accept a mounted parent target as long as it is not just root (/).
  mount_target="$(findmnt -T "$BITCOIN_MOUNT_PATH" -n -o TARGET 2>/dev/null || true)"
  if [[ -z "$mount_target" || "$mount_target" == "/" ]]; then
    echo "Required drive is not mounted: $BITCOIN_MOUNT_PATH" >&2
    exit 1
  fi
fi

if [[ "$DRY_RUN" -eq 0 && ! -x "$PARSER_BIN" ]]; then
  echo "Parser binary not executable: $PARSER_BIN" >&2
  exit 1
fi

if ! [[ "$STEP" =~ ^[0-9]+$ && "$STEP" -gt 0 ]]; then
  echo "--step must be a positive integer" >&2
  exit 1
fi

if ! [[ "$LOOPS" =~ ^[0-9]+$ && "$LOOPS" -gt 0 ]]; then
  echo "--loops must be a positive integer" >&2
  exit 1
fi

if ! [[ "$MAX_BLOCKS" =~ ^[0-9]+$ && "$MAX_BLOCKS" -gt 0 ]]; then
  echo "--max-blocks must be a positive integer" >&2
  exit 1
fi

DRY_RUN_MAX=""

for ((i=1; i<=LOOPS; i++)); do
  if [[ "$DRY_RUN" -eq 1 && -n "$DRY_RUN_MAX" ]]; then
    current_max="$DRY_RUN_MAX"
  else
    current_max=$(python3 - "$CONFIG" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as f:
    data = json.load(f)
print(int(data["parser"]["maxBlockNum"]))
PY
)
  fi

  if [[ "$current_max" -ge "$MAX_BLOCKS" ]]; then
    echo "[loop $i/$LOOPS] current maxBlockNum ($current_max) already at/above cap ($MAX_BLOCKS). Stopping."
    break
  fi

  next_max=$((current_max + STEP))
  if [[ "$next_max" -gt "$MAX_BLOCKS" ]]; then
    next_max="$MAX_BLOCKS"
  fi

  echo "[loop $i/$LOOPS] maxBlockNum: $current_max -> $next_max"

  if [[ "$DRY_RUN" -eq 1 ]]; then
    DRY_RUN_MAX="$next_max"
    continue
  fi

  python3 - "$CONFIG" "$next_max" <<'PY'
import json
import os
import sys

path = sys.argv[1]
new_max = int(sys.argv[2])

with open(path, "r", encoding="utf-8") as f:
    data = json.load(f)

if "parser" not in data:
    data["parser"] = {}
data["parser"]["maxBlockNum"] = new_max

tmp_path = f"{path}.tmp"
with open(tmp_path, "w", encoding="utf-8") as f:
    json.dump(data, f, indent=4)
    f.write("\n")
os.replace(tmp_path, path)
PY

  ts=$(date +%F_%H%M%S)
  update_log="update-${ts}-to-${next_max}.log"
  run_parser_step "[loop $i/$LOOPS] running update" "$update_log" update

  ts=$(date +%F_%H%M%S)
  addr_log="address-index-update-${ts}-to-${next_max}.log"
  run_parser_step "[loop $i/$LOOPS] running address-index-update" "$addr_log" address-index-update

  if [[ "$STOP_REQUESTED" -eq 1 ]]; then
    echo "Stop requested. Exiting after completed loop $i/$LOOPS."
    break
  fi
done

echo "Done."
