#include "myNRF24.h"
#include <esp_log.h>
#include "freertos/task.h"
#include <string.h>

NRF24_t dev;
bool piano_receive = false;
bool piano_finished = false;
bool ready_for_laser = false;

NRF24_t nrf24_init(){
    printf("Start NRF24 Init\n");
    Nrf24_init(&dev);
    uint8_t payload = 32;
    uint8_t channel = CONFIG_RADIO_CHANNEL;
    Nrf24_SetSpeedDataRates(&dev, 1);
    Nrf24_setRetransmitDelay(&dev, 3);
    Nrf24_SetOutputRF_PWR(&dev, 3);
    Nrf24_config(&dev, channel, payload);

    //Set the receiver address using 5 characters
    esp_err_t ret = Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK) {
        ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
        while(1) { vTaskDelay(1); }
    }

    //Print settings
    Nrf24_printDetails(&dev);
    printf("Done NRF24 Init\n");
    return dev;
}

void sender(void *pvParameters)
{
    //TODO need to be closed after piano game has finished
    uint8_t buf[32];
    while(1) {
        TickType_t nowTick = xTaskGetTickCount();
        sprintf((char *)buf, "Hello World %"PRIu32, nowTick);
        Nrf24_send(&dev, buf);
        vTaskDelay(1);
        ESP_LOGI(pcTaskGetName(0), "Wait for sending.....");
        if (Nrf24_isSend(&dev, 1000)) {
            ESP_LOGI(pcTaskGetName(0),"Send success:%s", buf);
            piano_receive = true;
        } else {
            ESP_LOGW(pcTaskGetName(0),"Send fail:%s", buf);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void receiver(void *pvParameters)
{
    //TODO need to be closed after piano game has finished
    uint8_t buf[32];

    // Clear RX FiFo
    while(1) {
        if (Nrf24_dataReady(&dev) == false) break;
        Nrf24_getData(&dev, buf);
    }

    while(1) {
        //When the program is received, the received data is output from the serial port
        if (Nrf24_dataReady(&dev)) {
            Nrf24_getData(&dev, buf);
            ESP_LOGI(pcTaskGetName(0), "Got data:%s", buf);
            if(strncmp((char*)buf, "Piano Finished", 14) != 0)  piano_finished = true;
            if(strncmp((char*)buf, "ready_for_laser", 14) != 0) ready_for_laser = true;
            //ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), buf, payload, ESP_LOG_INFO);
        }
        vTaskDelay(1);
    }
}

esp_err_t notifyPiano(){
    esp_err_t ret;
    ret = xTaskCreate(&sender, "SENDER", 1024*3, NULL, 2, NULL);
    if(ret != ESP_OK)    return ret;
    while (1){
        if(piano_receive)   break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    return ret;
}

esp_err_t isPianoFinished(){
    esp_err_t ret;
    //TODO stop the task after we receive the signal of pianoFinished
    ret = xTaskCreate(&receiver, "RECEIVER", 1024*3, NULL, 2, NULL);
    if(ret != ESP_OK)   return ret;
    while (1)
        if(piano_finished)  break;
    return ret;
}

esp_err_t isReadyForLaser(){
    esp_err_t ret;
    //TODO stop the task after we receive the signal of pianoFinished
    ret = xTaskCreate(&receiver, "RECEIVER", 1024*3, NULL, 2, NULL);
    if(ret != ESP_OK)   return ret;
    while (1)
        if(ready_for_laser) break;
    return ret;
}