#include <esp_err.h>
#include "driver/gptimer.h"

esp_err_t speaker_init(gptimer_handle_t* timer_handle);
esp_err_t play_speaker_sine(int frequency, int amplitude, gptimer_handle_t* timer_handle);
esp_err_t play_speaker_audio(const char* file_name, gptimer_handle_t* timer_handle);