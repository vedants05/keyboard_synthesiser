#ifndef FILTERS_H
#define FILTERS_H

#include <stdint.h>

typedef enum {
    LPF,  // Low-pass filter
    HPF   // High-pass filter
} FilterType;
typedef struct {
    FilterType type;   // LPF or HPF
    float      fs;     // Sample rate
    float      cutoff; // Cutoff frequency 
    float      Q;      // Resonance 
    float b0, b1, b2;
    float a1, a2;
    float z1, z2;      // Delay states 
} Biquad;

// Function pointer type for filter setup
typedef void (*FilterSetupFn)(Biquad *filter, int sample_rate, float cutoff, float Q);

void high_pass_filter(int16_t *buffer, int bufferlen, float cutoff, float resonance, int sample_rate);
void low_pass_filter(int16_t *buffer, int bufferlen, float cutoff, float resonance, int sample_rate);
void apply_biquad_filter(int16_t *buffer, int bufferlen, FilterSetupFn setup, int sample_rate, 
                            float cutoff, float Q);

void init_biquad(Biquad *filter, int sample_rate, float cutoff, float Q);
void update_coefficients_biquad(Biquad *filter);
float process_sample(Biquad *filter, float x);

#endif // FILTERS_H
