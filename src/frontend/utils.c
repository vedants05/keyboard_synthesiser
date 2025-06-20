#include <stdio.h>

#include "utils.h"
#include "../backend/audio.h"
#include "../backend/filters.h"

//static double current_backend_frequency = 440.0;
static double current_backend_volume = 0.8;

static waveform current_waveform_type = GUI_SINE; //DEFAULT TO SINE
static filter_type current_filter_type = GUI_NO_FILTER; //DEFAULT TO NO FILTER

// Global synth context for audio backend
static SynthContext *global_synth_ctx = NULL;

void init_audio_backend(SynthContext *ctx) {
    global_synth_ctx = ctx;
}

void set_synth_frequency(double freq) {
    if (freq < 20.0) freq = 20.0; // Min frequency
    if (freq > 20000.0) freq = 20000.0; // Max frequency (increased range for keyboard)
    //current_backend_frequency = freq;
    
    // Update the actual audio backend
    if (global_synth_ctx) {
        global_synth_ctx->frequency = freq;
    }
}

void set_synth_volume(double vol) {
    if (vol < 0.0) vol = 0.0;
    if (vol > 1.0) vol = 1.0;
    current_backend_volume = vol;
    
    // Update the actual audio backend
    if (global_synth_ctx) {
        global_synth_ctx->current_amplitude = (int16_t)(vol * global_synth_ctx->max_amplitude);
    }
}

void set_synth_waveform(waveform waveform_type) {
    current_waveform_type = waveform_type;
    
    // Update the actual audio backend
    if (global_synth_ctx) {
        switch (waveform_type) {
            case GUI_SINE:
                global_synth_ctx->wave_type = SINE;
                break;
            case GUI_SQUARE:
                global_synth_ctx->wave_type = SQUARE;
                break;
            case GUI_SAW:
                global_synth_ctx->wave_type = SAW;
                break;
        }
    }
}

void set_synth_filter_type(filter_type filter) {
    current_filter_type = filter;
    
    // Update the actual audio backend filter
    if (global_synth_ctx && global_synth_ctx->filter) {
        switch (filter) {
            case GUI_NO_FILTER:
                global_synth_ctx->filter->type = NO_FILTER;
                break;
            case GUI_LOW_FILTER:
                global_synth_ctx->filter->type = LPF;
                break;
            case GUI_HIGH_FILTER:
                global_synth_ctx->filter->type = HPF;
                break;
        }
    }
}

// Function to tell the backend to stop playing sound (for monophonic playback)
void stop_synth_note() {
    if (global_synth_ctx) {
        global_synth_ctx->current_amplitude = 0;
    }
}

// Function to start a note at a given frequency
void start_synth_note(double freq, double vol) {
    set_synth_frequency(freq);
    set_synth_volume(vol);
}


// double get_current_frequency() {
//     return current_backend_frequency;
// }

double get_current_volume() {
    return current_backend_volume;
}

waveform get_current_waveform_type() {
    return current_waveform_type;
}

filter_type get_current_filter_type() {
    return current_filter_type;
}

