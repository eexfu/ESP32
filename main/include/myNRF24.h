#include "mirf.h"
NRF24_t nrf24_init();
void sender(void *pvParameters);
void receiver(void *pvParameters);
esp_err_t notifyPiano();
esp_err_t isPianoFinished();
esp_err_t isReadyForLaser();