#include "../component/mirf/mirf.h"
#include "driver/gptimer.h"

typedef struct {
    gptimer_handle_t* timer_handle;
    NRF24_t* dev;
} receiverParams_t;

esp_err_t nrf24_init(NRF24_t* dev);
void sender(void *pvParameters);
void receiver(void *pvParameters);
esp_err_t notifyPiano(NRF24_t* dev);
esp_err_t isPianoFinished(gptimer_handle_t* timer_handle, NRF24_t* dev);
esp_err_t isReadyForLaser(NRF24_t* dev);