#include "myNRF24.h"
#include <esp_log.h>
#include "freertos/task.h"
#include <string.h>
#include "mySpeaker.h"

bool piano_receive = false;
bool piano_finished = false;
bool ready_for_laser = false;

esp_err_t nrf24_init(NRF24_t* dev){
    printf("Start NRF24 Init\n");
    Nrf24_init(dev);
    uint8_t payload = 32;
    uint8_t channel = CONFIG_RADIO_CHANNEL;
    Nrf24_SetSpeedDataRates(dev, 1);
    Nrf24_setRetransmitDelay(dev, 3);
    Nrf24_SetOutputRF_PWR(dev, 3);
    Nrf24_config(dev, channel, payload);

    //Set the receiver address using 5 characters
    esp_err_t ret = Nrf24_setTADDR(dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK) {
        ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
        return ret;
//        while(1) { vTaskDelay(1); }
    }

    //Print settings
    Nrf24_printDetails(dev);
    printf("Done NRF24 Init\n");
    return ret;
}

void sender(void *pvParameters)
{
    //TODO need to stop after finished
    NRF24_t dev = *(NRF24_t *)pvParameters;
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
//            vTaskDelete(NULL);
        } else {
            ESP_LOGW(pcTaskGetName(0),"Send fail:%s", buf);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void receiver(void *pvParameters)
{
    //TODO need to stop after finished
    receiverParams_t* params = (receiverParams_t*)pvParameters;
    NRF24_t dev = *(params->dev);
    gptimer_handle_t* timer_handle = params->timer_handle;
    uint8_t buf[32];

    // Clear RX FiFo
    while(1) {
        if (Nrf24_dataReady(&dev) == false) break;
        Nrf24_getData(&dev, buf);
    }

    int amplitude = 127;

    while(1) {
        //When the program is received, the received data is output from the serial port
        if (Nrf24_dataReady(&dev)) {
            Nrf24_getData(&dev, buf);
            ESP_LOGI(pcTaskGetName(0), "Got data:%s", buf);
            if(strncmp((char*)buf, "Piano Finished", 14) != 0){
                piano_finished = true;
//                vTaskDelete(NULL);
            }
            else if(strncmp((char*)buf, "ready_for_laser", 15) != 0){
                ready_for_laser = true;
//                vTaskDelete(NULL);
            }
            else if(strncmp((char*)buf, "do", 2) != 0) play_speaker_audio("../../res/piano_note/c4.txt", timer_handle);
            else if(strncmp((char*)buf, "re", 2) != 0) play_speaker_audio("../../res/piano_note/d4.txt", timer_handle);
            else if(strncmp((char*)buf, "mi", 2) != 0) play_speaker_audio("../../res/piano_note/e4.txt", timer_handle);
            else if(strncmp((char*)buf, "fa", 2) != 0) play_speaker_audio("../../res/piano_note/f4.txt", timer_handle);
            else if(strncmp((char*)buf, "so", 2) != 0) play_speaker_audio("../../res/piano_note/g4.txt", timer_handle);
            else if(strncmp((char*)buf, "la", 2) != 0) play_speaker_audio("../../res/piano_note/a4.txt", timer_handle);
            else if(strncmp((char*)buf, "xi", 2) != 0) play_speaker_audio("../../res/piano_note/b4.txt", timer_handle);
            else if(strncmp((char*)buf, "up_do", 2) != 0) play_speaker_audio("../../res/piano_note/c5.txt", timer_handle);
            else ;
            //ESP_LOG_BUFFER_HEXDUMP(pcTaskGetName(0), buf, payload, ESP_LOG_INFO);
        }
        vTaskDelay(1);
    }
}

esp_err_t notifyPiano(NRF24_t* dev){
    esp_err_t ret;
    ret = xTaskCreate(&sender, "SENDER", 1024*3, (void *)dev, 2, NULL);
    if(ret != ESP_OK)    return ret;
    while (1){
        if(piano_receive)   break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    return ret;
}

esp_err_t isPianoFinished(gptimer_handle_t* timer_handle, NRF24_t* dev){
    esp_err_t ret;
    receiverParams_t params = {
            .timer_handle = timer_handle,
            .dev = dev,
    };
    ret = xTaskCreate(&receiver, "RECEIVER", 1024*3, (void *)&params, 2, NULL);
    if(ret != ESP_OK)   return ret;
    while (1){
        if(piano_finished)  break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    return ret;
}

esp_err_t isReadyForLaser(NRF24_t* dev){
    esp_err_t ret;
    ret = xTaskCreate(&receiver, "RECEIVER", 1024*3, (void *)dev, 2, NULL);
    if(ret != ESP_OK)   return ret;
    while (1)
        if(ready_for_laser) break;
    return ret;
}