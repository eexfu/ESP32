#include "myNRF24.h"
#include <esp_log.h>
#include "freertos/task.h"
#include <string.h>
#include "mySpeaker.h"

bool piano_receive = false;
bool piano_finished = false;
bool ready_for_laser = false;
uint8_t payload = 2;

esp_err_t nrf24_init(NRF24_t* dev){
    printf("Start NRF24 Init\n");
    Nrf24_init(dev);
    uint8_t channel = CONFIG_RADIO_CHANNEL;
    Nrf24_config(dev, channel, payload);

    esp_err_t ret;
    Nrf24_setTADDR(dev, (uint8_t *)"ABCDE");
    ret = Nrf24_setRADDR(dev, (uint8_t *)"FGHIJ");
    if (ret != ESP_OK) {
        ESP_LOGE(pcTaskGetName(0), "nrf24l01 not installed");
//        return ret;
        while(1){ vTaskDelay(1); }
    }

    Nrf24_SetSpeedDataRates(dev, 0);
    Nrf24_setRetransmitDelay(dev, 3);

    //Print settings
    Nrf24_printDetails(dev);
    printf("Done NRF24 Init\n");
    return ret;
}

void sender(void *pvParameters)
{
    NRF24_t dev = *(NRF24_t *)pvParameters;
    uint8_t buf[32];
    while(1) {
//        TickType_t nowTick = xTaskGetTickCount();
//        sprintf((char *)buf, "Hello World %"PRIu32, nowTick);
        buf[0] = 0;
        buf[1] = 99;
        Nrf24_send(&dev, buf);
        vTaskDelay(1);
        ESP_LOGI(pcTaskGetName(0), "Wait for sending.....");
        if (Nrf24_isSend(&dev, 1000)) {
            ESP_LOGI(pcTaskGetName(0),"Send success:%s", buf);
            piano_receive = true;
            vTaskDelete(NULL);
        } else {
            ESP_LOGW(pcTaskGetName(0),"Send fail:%s", buf);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void receiver(void *pvParameters)
{
    receiverParams_t *params = (receiverParams_t*)pvParameters;
    NRF24_t dev = *(params->dev);
    gptimer_handle_t* timer_handle = params->timer_handle;
    uint8_t buf[payload];

    // Clear RX FiFo
    while(1) {
        if (Nrf24_dataReady(&dev) == false) break;
        Nrf24_getData(&dev, buf);
    }

    printf("start receiver\n");
    while(1) {
        if (Nrf24_dataReady(&dev)) {
            Nrf24_getData(&dev, buf);
            ESP_LOGI(pcTaskGetName(0), "Got data:%d-%d", buf[0], buf[1]);
            if(buf[0] == 3 && buf[1] == 231){
                piano_finished = true;
                printf("piano_finished\n");
                vTaskDelete(NULL);
            }
//            else if(buf[0] == 11){
//                ready_for_laser = true;
//                printf("ready for laser\n");
//                vTaskDelete(NULL);
//            }
            else if(buf[0] == 1 && buf[1] == 06) play_speaker_audio("/spiffs/c4.txt", timer_handle);
            else if(buf[0] == 1 && buf[1] == 38) play_speaker_audio("/spiffs/d4.txt", timer_handle);
            else if(buf[0] == 1 && buf[1] == 74) play_speaker_audio("/spiffs/e4.txt", timer_handle);
            else if(buf[0] == 1 && buf[1] == 93) play_speaker_audio("/spiffs/f4.txt", timer_handle);
            else if(buf[0] == 1 && buf[1] == 136) play_speaker_audio("/spiffs/g4.txt", timer_handle);
            else if(buf[0] == 1 && buf[1] == 184) play_speaker_audio("/spiffs/a4.txt", timer_handle);
            else if(buf[0] == 1 && buf[1] == 238) play_speaker_audio("/spiffs/b4.txt", timer_handle);
            else if(buf[0] == 2 && buf[1] == 11) play_speaker_audio("/spiffs/c5.txt", timer_handle);
            else play_speaker_audio("/spiffs/sine.txt", timer_handle);
        }
        vTaskDelay(1);
    }
}

esp_err_t notifyPiano(NRF24_t* dev){
    esp_err_t ret;
    ret = xTaskCreate(&sender, "SENDER", 1024*3, (void *)dev, 2, NULL);
    if(ret != ESP_OK){
        ESP_LOGI("NRF24", "create sender: %d", ret);
//        return ret;
    }
    while (1){
        if(piano_receive == true){
            ESP_LOGI("NRF24", "piano_receive: %s", (piano_receive)? "true":"false");
            break;
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    ESP_LOGI("NRF24", "%d", ret);
    return ESP_OK;
}

esp_err_t isPianoFinished(gptimer_handle_t* timer_handle, NRF24_t* dev){
    esp_err_t ret;
    receiverParams_t params = {
            .timer_handle = timer_handle,
            .dev = dev,
    };
    printf("Start Listening\n");
    ret = xTaskCreate(&receiver, "RECEIVER", 1024*3, (void *)&params, 2, NULL);
    if(ret != ESP_OK){
        ESP_LOGI("NRF24", "create receiver: %d", ret);
//        return ret;
    }
    while (1){
        if(piano_finished)  break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    return ESP_OK;
}

esp_err_t isReadyForLaser(NRF24_t* dev){
    esp_err_t ret = ESP_OK;
//    ret = xTaskCreate(&receiver, "RECEIVER", 1024*3, (void *)dev, 2, NULL);
//    if(ret != ESP_OK)   return ret;
//    while (1)
//        if(ready_for_laser) break;
    return ret;
}