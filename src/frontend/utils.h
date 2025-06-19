#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>

void set_synth_frequency(double freq);
void set_synth_volume(double vol);
void set_synth_waveform(int waveform_type);
void stop_synth_note();
void start_synth_note(double freq, double vol);
double get_current_frequency();
double get_current_volume();
int get_current_waveform_type();

#endif