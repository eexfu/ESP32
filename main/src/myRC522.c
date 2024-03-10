#include "../include/myRC522.h"
#include <esp_log.h>
#include "driver/mcpwm_prelude.h"
#include "include/myServo.h"

static const char* TAG = "RC522";
static const gpio_num_t buzzer_pin = GPIO_NUM_4;
static const gpio_num_t rc522_reset_pin = GPIO_NUM_6;
bool correct_key = false;
bool restart_key = false;

static void rc522_handler(void* arg, esp_event_base_t base, int32_t event_id, void* event_data){
    rc522_event_data_t* data = (rc522_event_data_t*) event_data;
    switch(event_id) {
        case RC522_EVENT_TAG_SCANNED: {
            rc522_tag_t* tag = (rc522_tag_t*) data->ptr;
            ESP_LOGI(TAG, "Tag scanned (sn: %" PRIu64 ")", tag->serial_number);
            if(tag->serial_number == 232215100675){
                printf("correct key\n");
                correct_key = true;
                fflush(stdout);
                rotate_servo(90);
            } else if(tag->serial_number == 787332672355){
                printf("wrong key\n");
                fflush(stdout);
                open_buzzer();
                vTaskDelay(pdMS_TO_TICKS(1000));
                close_buzzer();
                rotate_servo(-90);
            }
            else if(tag->serial_number == 000000000000){
                //TODO need to decide the serial number of reset key later
                printf("restart key\n");
                fflush(stdout);
                restart_key = true;
                rotate_servo(-90);
            }
            else{
                printf("unknown key\n");
                fflush(stdout);
                open_buzzer();
                vTaskDelay(pdMS_TO_TICKS(1000));
                close_buzzer();
                rotate_servo(-90);
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

esp_err_t buzzer_init() {
    esp_err_t ret;
    gpio_set_direction(buzzer_pin, GPIO_MODE_OUTPUT);
    ret = gpio_set_level(buzzer_pin, 0);
    return ret;
}

void open_buzzer(){
    gpio_set_level(buzzer_pin, 1);
}

void close_buzzer(){
    gpio_set_level(buzzer_pin, 0);
}

esp_err_t rc522_init(rc522_handle_t* scanner){
    printf("Start RC522 Init\n");
    //reset rc522
    gpio_set_level(rc522_reset_pin,1);

    rc522_config_t config = {
            .spi.host = SPI3_HOST,
            .spi.sda_gpio = 47,
            .spi.bus_is_initialized = true,
    };

    esp_err_t ret;
    ret = rc522_create(&config, scanner);
    if(ret != ESP_OK)   return ret;
    if(scanner != NULL){
        printf("scanner is not null\n");
    } else{
        printf("scanner is null\n");
    }
    ret = rc522_register_events(*scanner, RC522_EVENT_ANY, rc522_handler, NULL);
    printf("Done RC522 Init\n");
    return ret;
}

esp_err_t myRC522_start(rc522_handle_t* scanner){
    esp_err_t ret;
    ret = rc522_start(*scanner);
    if(ret != ESP_OK)   return ret;
    while(1){
        if(correct_key) break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    ret = rc522_pause(*scanner);
    return ret;
}

esp_err_t detectRestartTag(rc522_handle_t* scanner){
    esp_err_t ret;
    ret = rc522_start(*scanner);
    if(ret != ESP_OK)   return ret;
    while(1){
        if(restart_key) break;
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    ret = rc522_pause(*scanner);
    return ret;
}