#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>
#include "../backend/state.h"
#include "../backend/filters.h"

typedef enum {
    GUI_SINE,
    GUI_SQUARE,
    GUI_SAW,
} waveform;

typedef enum {
    GUI_NO_FILTER,
    GUI_LOW_FILTER,
    GUI_HIGH_FILTER,
} filter_type;

void init_audio_backend(SynthContext *ctx);
void set_synth_frequency(double freq);
void set_synth_volume(double vol);
void set_synth_waveform(waveform waveform_type);
void set_synth_filter_type(filter_type filter);
void stop_synth_note();
void start_synth_note(double freq, double vol);
double get_current_frequency();
double get_current_volume();

waveform get_current_waveform_type();
filter_type get_current_filter_type();

#endif
