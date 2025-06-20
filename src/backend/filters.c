#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "filters.h"


#ifndef M_PI // for Windows
#define M_PI 3.14159265358979323846
#endif

// Clamp a float value between lo and hi
static inline float clampf(float v, float lo, float hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

void init_synth_filter(Biquad *f, int sample_rate, FilterType type, float init_cutoff, float init_Q) {
    init_biquad(f, sample_rate, init_cutoff, init_Q);
    f->type = type;
    update_coefficients_biquad(f);
}

void set_filter_cutoff(Biquad *f, float cutoff) {
    f->cutoff = cutoff;
    update_coefficients_biquad(f);
}

void set_filter_resonance(Biquad *f, float Q) {
    f->Q = Q;
    update_coefficients_biquad(f);
}

void set_filter_type(Biquad *f, FilterType type) {
    f->type = type;
    update_coefficients_biquad(f);
}

void apply_biquad(Biquad *f, int16_t *buffer, int bufferlen) {
    for (int i = 0; i < bufferlen; ++i) {
        // Apply the Biquad filter to each sample in the buffer
        int16_t input = buffer[i];
        float x = (float)input / 32768.0f; // Normalize input to [-1.0, 1.0]
        float y = process_sample(f, x);
        y = clampf(y, -1.0f, +1.0f);
        int16_t out = (int16_t)(y * 32767.0f); // Scale back to original range
        buffer[i] = out;
    }
}

// Both filters use 2 pole structure
// So there is a -12 dB/octave roll-off above/below the cutoff frequency 
// Setup function for HPF or LPF Biquad filters
static void setup_lpf(Biquad *f, int fs, float cutoff, float Q) {
    init_biquad(f, fs, cutoff, Q);
    f->type = LPF;
    update_coefficients_biquad(f);
}

static void setup_hpf(Biquad *f, int fs, float cutoff, float Q) {
    init_biquad(f, fs, cutoff, Q);
    f->type = HPF;
    update_coefficients_biquad(f);
}


void init_biquad(Biquad *filter, int sample_rate, float cutoff, float Q) {
    filter->fs = sample_rate;
    filter->cutoff = cutoff;
    filter->Q = Q;
    filter->z1 = 0.0;
    filter->z2 = 0.0;
}

void update_coefficients_biquad(Biquad *filter) {
    double normalized_cutoff = (2.0 * M_PI * filter->cutoff) / filter->fs;
    double alpha = sin(normalized_cutoff) / (2.0 * filter->Q);
    double c = cos(normalized_cutoff);

    // Coefficients for the Biquad filter
    float a0 = 1.0 + alpha;

    // Normalize coefficients and add to the filter structure
    // Numerator coefficients (b0, b1, b2) differ slightly for LPF and HPF 
    if (filter->type == LPF) {
        filter->b0 = ((1.0 - c) / 2.0) / a0;
        filter->b1 = (1.0 - c) / a0;
        filter->b2 = ((1.0 - c) / 2.0) / a0;
    } else if (filter->type == HPF) {
        filter->b0 = ((1.0 + c) / 2.0) / a0;
        filter->b1 = -(1.0 + c) / a0;
        filter->b2 = ((1.0 + c) / 2.0) / a0;
    }
    
    // Denominator coefficients a1, a2 are the same for LPF and HPF
    filter->a1 = (-2.0 * c) / a0;
    filter->a2 = (1.0 - alpha) / a0;
}

float process_sample(Biquad *filter, float x) {
    // Apply the Biquad filter to a single sample
    float y = filter->b0 * x + filter->z1;
    
    // Update delay states
    filter->z1 = filter->b1 * x - filter->a1 * y + filter->z2;
    filter->z2 = filter->b2 * x - filter->a2 * y;

    return y;
}

// Allocate a Biquad on the heap for Python/ctypes
void *malloc_biquad(void) {
    return malloc(sizeof(Biquad));
}

// Free it later
void free_biquad(void *p) {
    free(p);
}
                        