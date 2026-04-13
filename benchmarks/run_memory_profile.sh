#!/bin/bash
mkdir -p build && cd build
cmake .. && make
echo "=== Static Footprint ==="
size ./ngac_benchmark
echo "=== Running Heap Profile ==="
valgrind --tool=massif --massif-out-file=massif.out ./ngac_benchmark
echo "=== Peak Heap Usage ==="
if command -v ms_print &> /dev/null; then
    ms_print massif.out | grep "MB" | head -n 20
else
    echo "ms_print not found (install valgrind)."
fi
