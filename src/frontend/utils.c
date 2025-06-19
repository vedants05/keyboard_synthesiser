#include <stdio.h>

#include "utils.h"

static double current_backend_frequency = 440.0;
static double current_backend_volume = 0.5;

static waveform current_waveform_type = SINE; //DEFAULT TO SINE
static filter_type current_filter_type = NO_FILTER; //DEFAULT TO NO FILTER

void set_synth_frequency(double freq) {
    if (freq < 20.0) freq = 20.0; // Min frequency
    if (freq > 20000.0) freq = 20000.0; // Max frequency (increased range for keyboard)
    current_backend_frequency = freq;
    // In your real backend: Send 'freq' to your audio generation logic.
    // If your backend handles note-on/off, you'd send a note-on here for this frequency
    // printf("Backend: Setting frequency to %.2f Hz\n", freq);
}

void set_synth_volume(double vol) {
    if (vol < 0.0) vol = 0.0;
    if (vol > 1.0) vol = 1.0;
    current_backend_volume = vol;
    // In your real backend: Send 'vol' to your audio generation logic.
    // printf("Backend: Setting volume to %.2f\n", vol);
}

void set_synth_waveform(waveform waveform_type) {
    current_waveform_type = waveform_type;
    // In your real backend: Tell your audio generation logic to switch waveform.
    // printf("Backend: Setting waveform to %s\n", (waveform_type == 0) ? "Sine" : "Square");
}

void set_filter_type(filter_type filter) {
    current_filter_type = filter;
}

// Function to tell the backend to stop playing sound (for monophonic playback)
void stop_synth_note() {
    // In your real backend: Tell your audio generation logic to stop current note.
    // This might mean setting amplitude to 0 or sending a note-off event.
    // For our simple backend, we'll set volume to 0.
    set_synth_volume(0.0);
    // printf("Backend: Stopping note.\n");
}

// Function to start a note at a given frequency
void start_synth_note(double freq, double vol) {
    set_synth_frequency(freq);
    set_synth_volume(vol); // Set volume to actual desired volume
    // printf("Backend: Starting note at %.2f Hz with volume %.2f\n", freq, vol);
}


double get_current_frequency() {
    return current_backend_frequency;
}

double get_current_volume() {
    return current_backend_volume;
}

waveform get_current_waveform_type() {
    return current_waveform_type;
}

filter_type get_current_filter_type() {
    return current_filter_type;
}