//
// Created by desto on 27/03/2024.
//

#include "myIRSensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/gpio.h>

#define EM_GPIO_PIN 45

esp_err_t Electromagnet_init() {

    ESP_ERROR_CHECK( gpio_set_direction(EM_GPIO_PIN, GPIO_MODE_OUTPUT) );
    ESP_ERROR_CHECK( gpio_set_level(EM_GPIO_PIN, 1) );

    return ESP_OK;
}

esp_err_t setEM(bool level) {

    return gpio_set_level(EM_GPIO_PIN, level);
}