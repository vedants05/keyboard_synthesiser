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
lib.malloc_biquad.restype = ctypes.c_void_p
lib.free_biquad.argtypes = [ctypes.c_void_p]
lib.init_synth_filter.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_float, ctypes.c_float]
lib.init_synth_filter.restype = None
lib.apply_biquad.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_int16), ctypes.c_int]
lib.apply_biquad.restype = None

# Helpers to convert numpy arrays to C buffers
def make_int16_buffer(signal: np.ndarray):
    arr = np.ascontiguousarray(signal * 32767, dtype=np.int16)
    buf = (ctypes.c_int16 * arr.size)(*arr.flatten())
    return buf, arr.size

def int16_buffer_to_float(buf, length: int):
    raw = np.frombuffer(buf, count=length, dtype=np.int16)
    return raw.astype(np.float32) / 32767.0

# Filter parameters
fs     = 48000
cutoff = 1000.0
Q      = 0.707
LPF_TYPE = 0  # matches C enum

# Using impulse response to generate a smooth frequency curve
N = 8192
impulse = np.zeros(N, dtype=np.float32)
impulse[0] = 1.0

# Initialize the filter in C
bq = lib.malloc_biquad()
lib.init_synth_filter(bq, fs, LPF_TYPE, cutoff, Q)

# Apply LPF filter to the impulse
buf, length = make_int16_buffer(impulse)
lib.apply_biquad(bq, buf, length)
imp_resp = int16_buffer_to_float(buf, length)

lib.free_biquad(bq)

# Compute frequency response via FFT of impulse response
freqs = np.fft.rfftfreq(N, d=1/fs)
mag   = 20 * np.log10(np.abs(np.fft.rfft(imp_resp)) + 1e-12)

# High pass filter on the same impulse response
HPF_TYPE = 1

# initialize C biquad for HPF
bq_hp = lib.malloc_biquad()
lib.init_synth_filter(bq_hp, fs, HPF_TYPE, cutoff, Q)

# apply HPF to the same impulse
buf_hp, _ = make_int16_buffer(impulse)
lib.apply_biquad(bq_hp, buf_hp, length)
hp_imp_resp = int16_buffer_to_float(buf_hp, length)

lib.free_biquad(bq_hp)

# compute its frequency response
hp_mag = 20 * np.log10(np.abs(np.fft.rfft(hp_imp_resp)) + 1e-12)

# overlay on the same figure
fig1 = plt.figure(figsize=(10, 5))
mgr = fig1.canvas.manager
if hasattr(mgr, "window"):
    mgr.window.wm_geometry("+0+0")
plt.semilogx(freqs, mag,    label=f"LPF : (fc={cutoff} Hz, Q={Q})")
plt.semilogx(freqs, hp_mag, label=f"HPF : (fc={cutoff} Hz, Q={Q})")
plt.axvline(cutoff, color='k', linestyle='--', linewidth=1, label="Cutoff frequency")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude (dB)")
plt.title("LPF vs HPF on a sound curve")
plt.legend()
plt.grid(True, which='both', linestyle=':', linewidth=0.5)
plt.show(block=False)

# ─── Time-Domain Chirp Test for LPF & HPF ─────────────────────────────────


# Duration & time axis
T_chirp = 0.05                # 50 ms
n_chirp = int(fs * T_chirp)
t_chirp = np.linspace(0, T_chirp, n_chirp, endpoint=False)

# Linear chirp from f0→f1
f0, f1 = 100.0, 10000.0       # start/end freqs
k = (f1 - f0) / T_chirp       # chirp rate
phase = 2*np.pi*(f0*t_chirp + 0.5*k*t_chirp**2)
chirp = np.sin(phase)

# — Apply LPF to chirp —
bq_l = lib.malloc_biquad()
lib.init_synth_filter(bq_l, fs, 0, cutoff, Q)    # 0 = LPF
buf_l, _ = make_int16_buffer(chirp)
lib.apply_biquad(bq_l, buf_l, n_chirp)
lpf_chirp = int16_buffer_to_float(buf_l, n_chirp)
lib.free_biquad(bq_l)

# — Apply HPF to chirp —
bq_h = lib.malloc_biquad()
lib.init_synth_filter(bq_h, fs, 1, cutoff, Q)    # 1 = HPF
buf_h, _ = make_int16_buffer(chirp)
lib.apply_biquad(bq_h, buf_h, n_chirp)
hpf_chirp = int16_buffer_to_float(buf_h, n_chirp)
lib.free_biquad(bq_h)

# Zoom to show differences clearly
mid_idx = len(t_chirp) // 4

# Plot Original vs LPF for first half only
fig2 = plt.figure(figsize=(10, 3))
mgr2 = fig2.canvas.manager
if hasattr(mgr2, "window"):
    mgr2.window.wm_geometry("+0+600")
plt.plot(t_chirp[:mid_idx], chirp[:mid_idx],      label="Original Chirp", alpha=0.6)
plt.plot(t_chirp[:mid_idx], lpf_chirp[:mid_idx],  label="LPF Output",    linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.title("Original vs Low-Pass Filtered Chirp")
plt.legend()
plt.grid(True)
plt.show(block=False)

# Plot Original vs HPF for first half only
fig3 = plt.figure(figsize=(10, 3))
mgr3 = fig3.canvas.manager
if hasattr(mgr3, "window"):
    mgr3.window.wm_geometry("+700+600")
plt.plot(t_chirp[:mid_idx], chirp[:mid_idx],      label="Original Chirp", alpha=0.6)
plt.plot(t_chirp[:mid_idx], hpf_chirp[:mid_idx],  label="HPF Output",    linewidth=2)
plt.xlabel("Time (s)")
plt.ylabel("Amplitude")
plt.title("Original vs High-Pass Filtered Chirp")
plt.legend()
plt.grid(True)
plt.show(block=False)

if __name__ == "__main__":
    plt.show(block=True)


