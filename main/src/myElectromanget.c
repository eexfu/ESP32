//
// Created by desto on 27/03/2024.
//

#include "myIRSensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/gpio.h>

#define EM_GPIO_PIN 7

esp_err_t intit_electromagnet() {
    esp_err_t ret;
    ret = gpio_set_direction(EM_GPIO_PIN, GPIO_MODE_OUTPUT);
    if (ret != ESP_OK) return ret;
    ret = gpio_set_level(EM_GPIO_PIN, 1);
    if (ret != ESP_OK) return ret;
    return ESP_OK;
}

esp_err_t setEM(bool level) {
    return gpio_set_level(EM_GPIO_PIN, level);
}