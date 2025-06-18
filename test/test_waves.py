#!/usr/bin/env python3
import os
import sys
import ctypes
import numpy as np
import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

# Choose .dylib on macOS, .so on Linux
if sys.platform.startswith("darwin"):
    lib_ext = ".dylib"
else:
    lib_ext = ".so"

# Locate the shared library (build/synthcore.{so,dylib})
script_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(script_dir, os.pardir))
build_dir = os.path.join(project_root, "build")
lib_path = os.path.join(build_dir, f"synthcore{lib_ext}")

# Load the C filter library
lib = ctypes.CDLL(lib_path)

# C API
lib.malloc_synth_context.argtypes = [
    ctypes.c_int,    # sample_rate
    ctypes.c_double, # frequency
    ctypes.c_int,    # current_amplitude
    ctypes.c_int,    # max_amplitude
    ctypes.c_int     # wave_type
]
lib.malloc_synth_context.restype = ctypes.c_void_p

lib.free_synth_context.argtypes = [ctypes.c_void_p]
lib.free_synth_context.restype  = None

# Bind the three waveform generators
lib.generate_sine.argtypes   = [ctypes.c_void_p, ctypes.c_int]
lib.generate_sine.restype    = ctypes.POINTER(ctypes.c_int16)
lib.generate_square.argtypes = [ctypes.c_void_p, ctypes.c_int]
lib.generate_square.restype  = ctypes.POINTER(ctypes.c_int16)
lib.generate_saw.argtypes    = [ctypes.c_void_p, ctypes.c_int]
lib.generate_saw.restype     = ctypes.POINTER(ctypes.c_int16)

# Converting the returned int16_t* into a numpy array
def ptr_to_array(ptr, length, amplitude):
    arr_type = ctypes.c_int16 * length
    raw = ctypes.cast(ptr, ctypes.POINTER(arr_type)).contents
    np_arr = np.frombuffer(raw, dtype=np.int16).astype(np.float32)
    # normalize so peaks become ±1.0
    return np_arr / float(amplitude)

# Test parameters
fs        = 48000
frequency = 440.0
amplitude = 3000           # must match what you pass into C
duration  = 0.01           # 10 ms
N         = int(fs * duration)
t         = np.linspace(0, duration, N, endpoint=False)

WAVE_SINE   = 0
WAVE_SQUARE = 1
WAVE_SAW    = 2

# Generate & graph each waveform
for i, (name, wave_type, generator) in enumerate([
    ("Sine",   WAVE_SINE,   lib.generate_sine),
    ("Square", WAVE_SQUARE, lib.generate_square),
    ("Saw",    WAVE_SAW,    lib.generate_saw),
]):
    ctx = lib.malloc_synth_context(fs, frequency, amplitude, amplitude, wave_type)
    
    ptr  = generator(ctx, N)
    data = ptr_to_array(ptr, N, amplitude)
    
    lib.free_synth_context(ctx)

    fig = plt.figure(figsize=(8, 4))
    mgr = fig.canvas.manager
    if hasattr(mgr, "window"):
        # move each window 300 px to the right of the last
        x_offset = i * 400
        y_offset = i * 250
        mgr.window.wm_geometry(f"+{x_offset}+{y_offset}")

    # reference waveform plotted using their respective mathematical formula
    if wave_type == WAVE_SINE:
        ref = np.sin(2 * np.pi * frequency * t)
    elif wave_type == WAVE_SAW:
        ref = 2 * ((frequency * t) % 1) - 1
    else:  # square
        ref = np.sign(np.sin(2 * np.pi * frequency * t))

    plt.plot(
        t, data,
        linestyle="-",
        color="blue",
        linewidth=2,
        label="Generated"
    )

    plt.plot(
        t, ref,
        linestyle=(0, (5, 5)),
        color="red",
        linewidth=3,
        label="Reference"
    )

    plt.xlabel("Time (s)")
    plt.ylabel("Amplitude")
    plt.title(f"{name} Wave (440 Hz)")
    plt.legend()
    plt.grid(True)

if __name__ == "__main__":
    plt.show()
