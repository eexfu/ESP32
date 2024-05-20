#include "../include/myRC522.h"
#include <esp_log.h>
#include "driver/mcpwm_prelude.h"
#include "include/myServo.h"
#include "mySpeaker.h"

static const char* TAG = "RC522";
static const gpio_num_t rc522_reset_pin = GPIO_NUM_21;
mcpwm_cmpr_handle_t comparator;
static const int rc522_servo_pin = 1;
bool correct_key = false;
bool restart_key = false;

static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data){
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;
    gptimer_handle_t* timer_handle = (gptimer_handle_t*) arg;
    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
            rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
            ESP_LOGI(TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
            if(tag->serial_number == 232215100675){
                printf("correct key\n");
                correct_key = true;
                fflush(stdout);
                vTaskDelay(pdMS_TO_TICKS(1000));
//                rotate_servo(90, &comparator);
                mcpwm_timer_handle_t timer = servo_init(rc522_servo_pin, &comparator);
                vTaskDelay(pdMS_TO_TICKS(1500));
//                rotate_servo(90, comparator);
                rotate_servo(90, &comparator);
                mcpwm_timer_start_stop(timer, MCPWM_TIMER_STOP_FULL);
                mcpwm_timer_disable(timer);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
            else if(tag->serial_number == 787332672355){
                //TODO need to decide the serial number of reset key later
                printf("restart key\n");
                fflush(stdout);
                restart_key = true;
//                rotate_servo(-90, &comparator);
            }
            else{
                printf("wrong key\n");
                fflush(stdout);
//                play_speaker_sine(1000, 127, timer_handle);
                play_speaker_audio("/spiffs/sine.txt", timer_handle);
                vTaskDelay(pdMS_TO_TICKS(1000));
//                rotate_servo(-90, &comparator);
            }
        }
            break;
    }
}

esp_err_t rc522_power_down() {
    esp_err_t ret;
    gpio_set_direction(rc522_reset_pin, GPIO_MODE_OUTPUT);
    ret = gpio_set_level(rc522_reset_pin,0);
    return ret;
}

esp_err_t rc522_power_up(){
    esp_err_t ret;
    //reset rc522
    ret = gpio_set_level(rc522_reset_pin,1);
    return ret;
}

esp_err_t rc522_init(rc522_handle_t* scanner, gptimer_handle_t* timer_handle){
    printf("Start RC522 Init\n");

    rc522_power_up();

    rc522_config_t config = {
            .spi.host = SPI3_HOST,
            .spi.sda_gpio = 47,
            .spi.bus_is_initialized = true,
    };

    //used for debug if we didn't pre-initialize SPI
//    rc522_config_t config = {
//            .spi.host = SPI3_HOST,
//            .spi.miso_gpio = 13,
//            .spi.mosi_gpio = 11,
//            .spi.sck_gpio = 12,
//            .spi.sda_gpio = 47,
//    };

    esp_err_t ret;

//    ret = servo_init(rc522_servo_pin, &comparator);
//    if(ret != ESP_OK)   return ret;


    ret = rc522_create(&config, scanner);
    if(ret != ESP_OK)   return ret;
    if(scanner != NULL){
        printf("scanner is not null\n");
    } else{
        printf("scanner is null\n");
    }
    ret = rc522_register_events(*scanner, RC522_EVENT_ANY, rc522_handler, (void*)timer_handle);
    rc522_power_down();
    printf("Done RC522 Init\n");
    return ret;
}

esp_err_t myRC522_start(rc522_handle_t* scanner){
    esp_err_t ret;
    rc522_power_up();
    ret = rc522_start(*scanner);
    if(ret != ESP_OK)   return ret;
    while(1){
        if(correct_key) break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    rc522_pause(*scanner);
    ret = rc522_power_down();
    return ret;
}

esp_err_t detectRestartTag(rc522_handle_t* scanner){
    esp_err_t ret;
    rc522_power_up();
    ret = rc522_start(*scanner);
    if(ret != ESP_OK){
        ESP_LOGI(TAG ,"restart tag return ret");
//        return ret;
    }
    while(1){
        if(restart_key) break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    ret = rc522_pause(*scanner);
    return ret;
}