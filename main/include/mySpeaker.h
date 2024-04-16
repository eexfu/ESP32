#include <esp_err.h>
#include "driver/gptimer.h"

esp_err_t speaker_init(gptimer_handle_t* timer_handle);
esp_err_t speaker_init_audio(gptimer_handle_t* timer_handle);
esp_err_t play_speaker_sine(int frequency, int amplitude, gptimer_handle_t* timer_handle);
esp_err_t play_speaker_audio(char* file_name, gptimer_handle_t* timer_handle);
esp_err_t play_speaker_audio_pass(char* file_name, gptimer_handle_t* timer_handle);
esp_err_t play_audio(char* file_name, gptimer_handle_t* timer_handle);
esp_err_t play_sine_start(int frequency, int amplitude, gptimer_handle_t* timer_handle);
esp_err_t play_sine_stop(gptimer_handle_t* timer_handle);
void set_sine_wave(int frequency, int amplitude);