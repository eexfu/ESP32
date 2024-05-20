/*
 * Code based on oneshot-read from esp-idf-v5.2.1 framework, 2022-2023 Espressif Systems (Shanghai) CO LTD
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/index.html
 */

#include "./include/myLaser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/soc_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

const static char *TAG = "LDR_ADC";

/*---------------------------------------------------------------
        ADC General Macros (header file?)
---------------------------------------------------------------*/
#define LDR_ADC                 ADC_CHANNEL_1

#define LDR_ADC_ATTEN           ADC_ATTEN_DB_12 //For range to 3.1V


static int adc_raw[10];
static int voltage[10];

static bool
example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void example_adc_calibration_deinit(adc_cali_handle_t handle);

#define LASER_SAMPLE_PERIOD            100

/*---------------------------------------------------------------
        Process data
---------------------------------------------------------------*/
#define SAMPLES_THRESHOLD 10
#define THRESHOLD_LDR 2500

bool samplingDone = false;
int thresholdArray[SAMPLES_THRESHOLD];
int threshold = 0;
int countConsec = 0;
int count = 0;

bool processLaserData(int value) {
    ESP_LOGI(TAG, "value= %d, threshold %d, countConsec = %d", value, threshold, countConsec);

    if (!samplingDone) {
        thresholdArray[count] = value;
        count++;
        if (count >= SAMPLES_THRESHOLD) {
            int sum = 0;
            for (int i = 0; i < sizeof(thresholdArray); i++) {
                sum += thresholdArray[i];
            }
            threshold = sum / sizeof(thresholdArray);
            samplingDone = true;
        }

    } else {

        if (value > threshold) {
                countConsec++;
                if (countConsec >= 10) {
                    return true;
                }
            } else {
                countConsec = 0;
            }

    }

    return false;
}

/*---------------------------------------------------------------
        Main function
---------------------------------------------------------------*/

bool laserSensorMain() {
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
            .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = LDR_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, LDR_ADC, &config));

    //-------------ADC1 Calibration Init---------------//
    adc_cali_handle_t adc1_cali_chan0_handle = NULL;
    bool do_calibration_LDR = example_adc_calibration_init(ADC_UNIT_1, LDR_ADC, LDR_ADC_ATTEN,
                                                          &adc1_cali_chan0_handle);

    //-------------LEDs Init---------------//
    //ESP_ERROR_CHECK(LEDs_init());

    bool retProcess = false;
    //-------------ADC1 Main Read Loop---------------//
    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, LDR_ADC, &adc_raw[0]));
        //ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, LDR_ADC, adc_raw[0]);

        if (do_calibration_LDR) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0], &voltage[0]));
            //ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, LDR_ADC, voltage[0]);
        }

        //ESP_LOGI(TAG, "Voltage: %d, \t raw voltage: %d", voltage[0], adc_raw[0]);
        retProcess = processLaserData(voltage[0]);

        if (retProcess == true) {
            break;
        } else {
            vTaskDelay(pdMS_TO_TICKS(LASER_SAMPLE_PERIOD));
        }

    }

    //-------------Teardown---------------------//
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    if (do_calibration_LDR) {
        example_adc_calibration_deinit(adc1_cali_chan0_handle);
    }

    return true;
}

/*---------------------------------------------------------------
        ADC Calibration
---------------------------------------------------------------*/
static bool
example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
                .unit_id = unit,
                .chan = channel,
                .atten = atten,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void example_adc_calibration_deinit(adc_cali_handle_t handle) {
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

}
