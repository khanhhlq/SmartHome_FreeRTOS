#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
mkdir -p tests/native/out
for f in $(find src -name '*.cpp' | sort); do
  echo "Checking $f"
  g++ -std=c++17 -fsyntax-only -Itests/native/include -Isrc -Wall -Wextra -Wno-unused-parameter "$f"
done
g++ -std=c++17 -Itests/native/include -Isrc tests/native/host_test.cpp src/core/app_state.cpp src/decision/decision.cpp -o tests/native/out/host_test
./tests/native/out/host_test
g++ -std=c++17 -Isrc tests/native/test_ldr_model.cpp -o tests/native/out/test_ldr_model
./tests/native/out/test_ldr_model
python tests/check_project.py
