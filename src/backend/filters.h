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

extern void init_synth_filter(Biquad *f, int sample_rate, FilterType type, float init_cutoff, float init_Q);

// Live updates from user input
extern void set_filter_cutoff(Biquad *f, float cutoff);
extern void set_filter_resonance(Biquad *f, float Q);
extern void apply_biquad(Biquad *f, int16_t *buffer, int bufferlen);

// Function pointer type for filter setup
typedef void (*FilterSetupFn)(Biquad *filter, int sample_rate, float cutoff, float Q);

void high_pass_filter(int16_t *buffer, int bufferlen, float cutoff, float resonance, int sample_rate);
void low_pass_filter(int16_t *buffer, int bufferlen, float cutoff, float resonance, int sample_rate);

void init_biquad(Biquad *filter, int sample_rate, float cutoff, float Q);
void update_coefficients_biquad(Biquad *filter);
float process_sample(Biquad *filter, float x);

// For Python/ctypes unit testing:
void *malloc_biquad(void);
void  free_biquad(void *p);

#endif // FILTERS_H
