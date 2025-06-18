#!/usr/bin/env bash
set -e

# 1) Build the C side (in build/)
mkdir -p build
cd build
cmake -DBUILD_KEYBOARD_TEST=OFF -DBUILD_SOUND_TEST=OFF -DBUILD_WAVE_TEST=OFF ..
cmake --build .

# 2) Go back to project root
cd ..

# 3) Activate your venv (if you’re using one)
if [ -f .venv/bin/activate ]; then
  # shellcheck source=/dev/null
  source .venv/bin/activate
fi

# 4) Run the Python filter‐test script (which calls plt.show())
python3 test/test_filters.py
