#include "../include/myDCMotor.h"
#include <stdio.h>
#include <driver/gpio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TAG "DC_Motor"


#define BDC_MCPWM_GPIO_A              4
#define BDC_MCPWM_GPIO_B              5
#define BDC_EN_GPIO                   6

esp_err_t Motor_init() {
    gpio_set_direction(BDC_MCPWM_GPIO_A, GPIO_MODE_OUTPUT);
    gpio_set_direction(BDC_MCPWM_GPIO_B, GPIO_MODE_OUTPUT);
    gpio_set_direction(BDC_EN_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(BDC_EN_GPIO, 0);
    gpio_set_level(BDC_MCPWM_GPIO_A, 0);
    gpio_set_level(BDC_MCPWM_GPIO_B, 0);


    return ESP_OK;
}

esp_err_t ElevatorMotor() {

    ESP_LOGI(TAG, "Motor start");

    gpio_set_level(BDC_EN_GPIO, 1);         //Motor Enable


    gpio_set_level(BDC_MCPWM_GPIO_A, 1);    //Motor forward
    gpio_set_level(BDC_MCPWM_GPIO_B, 0);

    vTaskDelay(pdMS_TO_TICKS(50));            //Motor on for short to get unstuck

    gpio_set_level(BDC_MCPWM_GPIO_A, 0);    //Motor Off
    gpio_set_level(BDC_MCPWM_GPIO_B, 0);

    vTaskDelay(pdMS_TO_TICKS(50));

    gpio_set_level(BDC_MCPWM_GPIO_A, 1);    //Motor forward
    gpio_set_level(BDC_MCPWM_GPIO_B, 0);

    vTaskDelay(pdMS_TO_TICKS(1000));           //Motor on for elevator rride

    gpio_set_level(BDC_MCPWM_GPIO_A, 0);    //Motor Off
    gpio_set_level(BDC_MCPWM_GPIO_B, 0);

    ESP_LOGI(TAG, "Motor end");

    return ESP_OK;
}



/*
//
// Created by desto on 13/04/2024.
//

#include "../include/myDCMotor.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/pulse_cnt.h"
#include "bdc_motor.h"
#include <driver/gpio.h>

#define TAG "DC_Motor"

#define BDC_MCPWM_TIMER_RESOLUTION_HZ 10000000 // 10MHz, 1 tick = 0.1us
#define BDC_MCPWM_FREQ_HZ             25000    // 25KHz PWM
#define BDC_MCPWM_DUTY_TICK_MAX       (BDC_MCPWM_TIMER_RESOLUTION_HZ / BDC_MCPWM_FREQ_HZ) // maximum value we can set for the duty cycle, in ticks
#define BDC_MCPWM_GPIO_A              4
#define BDC_MCPWM_GPIO_B              5

#define BDC_EN_GPIO                   6

#define TIME_FOR_ELEVATOR             400

esp_err_t ElevatorMotor() {

    //-------------DC-Motor Init---------------//
    ESP_LOGI(TAG, "Create DC motor");
    bdc_motor_config_t motor_config = {             //Motor config
            .pwm_freq_hz = BDC_MCPWM_FREQ_HZ,
            .pwma_gpio_num = BDC_MCPWM_GPIO_A,
            .pwmb_gpio_num = BDC_MCPWM_GPIO_B
    };

    bdc_motor_mcpwm_config_t mcpwm_config = {       //Motor control PWM config
            .group_id = 0,
            .resolution_hz = BDC_MCPWM_TIMER_RESOLUTION_HZ,
    };

    bdc_motor_handle_t motor = NULL;                //Makes motor handle

    ESP_LOGI(TAG, "New Motor");
    ESP_ERROR_CHECK(
            bdc_motor_new_mcpwm_device(&motor_config, &mcpwm_config, &motor)
            );

    ESP_LOGI(TAG, "Enable DC motor");
    bdc_motor_enable(motor);                        //idk what this does

    ESP_LOGI(TAG, "Forward DC motor");
    bdc_motor_forward(motor);

    //-------------EN-pin Init---------------//
    gpio_set_direction(BDC_EN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BDC_EN_GPIO, 1);


    //-------------Turn on for x seconds for elevator---------------//
    ESP_LOGI(TAG, "Turn Motor");
    bdc_motor_set_speed(motor, 200);
    vTaskDelay(pdMS_TO_TICKS(TIME_FOR_ELEVATOR));

    //-------------Turn of and teardown---------------//
    ESP_LOGI(TAG, "Coast");
    //bdc_motor_coast(motor);
    bdc_motor_set_speed(motor, 0);
    bdc_motor_disable(motor);
    vTaskDelay(pdMS_TO_TICKS(200));
    bdc_motor_del(motor);


    */
/* Find speed range (is from ~ 200 to ~ 400)
    esp_err_t err;
    for (int i = 1; i < 10000; i = i + 10) {
        vTaskDelay(pdMS_TO_TICKS(500));
        ESP_LOGI(TAG, "Set speed %d DC motor", i);
        err = bdc_motor_set_speed(motor, i);

        if (err != ESP_OK) {
            ESP_LOGI(TAG, "Speed %d gave error", i);
            vTaskDelay(pdMS_TO_TICKS(5000));
            break;
        }
    }
    *//*


    return ESP_OK; //TODO fix error handling
};*/
