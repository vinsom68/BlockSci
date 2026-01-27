#!/usr/bin/env sh
set -eu

TEST_BINARY="${1:-}"
TEST_CONFIG="${2:-}"

if [ -z "${TEST_BINARY}" ]; then
  echo "TEST_BINARY not set" >&2
  exit 1
fi

if [ -z "${TEST_CONFIG}" ]; then
  echo "BLOCKSCI_TEST_CONFIG not set; skipping blocksci_unittest" >&2
  exit 77
fi

if [ ! -f "${TEST_CONFIG}" ]; then
  echo "BLOCKSCI_TEST_CONFIG does not exist: ${TEST_CONFIG}" >&2
  exit 77
fi

exec "${TEST_BINARY}" "${TEST_CONFIG}"
