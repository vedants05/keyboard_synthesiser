#!/usr/bin/env bash
set -e

# Build the C side (in build/)
mkdir -p build
cd build
cmake -DBUILD_KEYBOARD_TEST=OFF -DBUILD_SOUND_TEST=OFF -DBUILD_WAVE_TEST=OFF ..
cmake --build .

# Go back to project root
cd ..

# Run the Python filter‐test script (which calls plt.show())
python3 test/test_filters.py
