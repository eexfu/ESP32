/*
 * Code based on oneshot-read from esp-idf-v5.2.1 framework, 2022-2023 Espressif Systems (Shanghai) CO LTD
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/index.html
 */

#include "myIRSensor.h"
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

const static char *TAG = "IR_ADC";
static const gpio_num_t solenoid_pin = GPIO_NUM_7;
/*---------------------------------------------------------------
        ADC General values
---------------------------------------------------------------*/
#define IR_ADC                 ADC_CHANNEL_7
#define IR_ADC_ATTEN           ADC_ATTEN_DB_11 //For range to 3.1V

#define IR_SAMPLE_PERIOD            (100)
#define MAX_COUNT                   (2000 / IR_SAMPLE_PERIOD)

static int adc_raw[10];

static int voltage[10];

gptimer_handle_t* timer_handle_speaker; //Speaker timer handle (this is the speaker that has been set up)

    static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
    static void example_adc_calibration_deinit(adc_cali_handle_t handle);
    //static esp_err_t adc_ir_init(adc_cali_handle_t handle);

    int convertToRange(int voltage);
    bool processIRData(int measuredVoltage);

    esp_err_t LEDs_init();
    esp_err_t LEDs_set_value(int value);

    esp_err_t solenoid_init();
    esp_err_t solenoid_set_value(bool value);

//
bool irSensorMain(gptimer_handle_t* timer_handle) {

    timer_handle_speaker = timer_handle;

    //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
            .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .atten = IR_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, IR_ADC, &config));

    //-------------ADC1 Calibration Init---------------//
    adc_cali_handle_t adc1_cali_chan0_handle = NULL;
    bool do_calibration_IR = example_adc_calibration_init(ADC_UNIT_1, IR_ADC, IR_ADC_ATTEN,
                                                          &adc1_cali_chan0_handle);

    //-------------LEDs Init---------------//
    ESP_ERROR_CHECK(LEDs_init());

    //-------------Solenoid Init---------------//
    ESP_ERROR_CHECK(solenoid_init());
    ESP_ERROR_CHECK(solenoid_set_value(1));

    bool retProcess;
    //-------------ADC1 Main Read Loop---------------//
    while (1) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, IR_ADC, &adc_raw[0]));
        //ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, IR_ADC, adc_raw[0]);

        if (do_calibration_IR) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_chan0_handle, adc_raw[0], &voltage[0]));
            //ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, IR_ADC, voltage[0]);
        }

        retProcess = processIRData(voltage[0]);

        if (retProcess == true) {
            break;
        } else {
            vTaskDelay(pdMS_TO_TICKS(IR_SAMPLE_PERIOD));
        }

    }

    //-------------Teardown---------------------//
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    if (do_calibration_IR) {
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
/*---------------------------------------------------------------
        LED control
---------------------------------------------------------------*/
//TODO integrate in rest of program

    const int IR_LEDS[] = {15, 16, 17, 18};

    esp_err_t LEDs_init() {
        for (int i = 0; i < sizeof(IR_LEDS) / sizeof(IR_LEDS[0]); i++) {
            ESP_ERROR_CHECK (gpio_set_direction(IR_LEDS[i], GPIO_MODE_OUTPUT) != ESP_OK);
            ESP_ERROR_CHECK (gpio_set_level(IR_LEDS[i], 0) != ESP_OK);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        return ESP_OK;
    }

    esp_err_t LEDs_set_value(int value) {
        int valueDisplays[5][4] = {{0, 0, 0, 0},
                                   {1, 0, 0, 0},
                                   {1, 1, 0, 0},
                                   {1, 1, 1, 0},
                                   {1, 1, 1, 1}};

        gpio_set_level(IR_LEDS[0], valueDisplays[value][0]);
        gpio_set_level(IR_LEDS[1], valueDisplays[value][1]);
        gpio_set_level(IR_LEDS[2], valueDisplays[value][2]);
        gpio_set_level(IR_LEDS[3], valueDisplays[value][3]);

        return ESP_OK; //TODO check each set level
    }


/*---------------------------------------------------------------
    Solenoid Control
---------------------------------------------------------------*/

    esp_err_t solenoid_int() {
        esp_err_t ret;
        ret = gpio_set_direction(solenoid_pin, GPIO_MODE_OUTPUT);
        solenoid_set_value(0);
        return ret;
    }

    esp_err_t solenoid_set_value(bool value){
        return gpio_set_level(solenoid_pin, value);
    }

/*---------------------------------------------------------------
        Data processing
---------------------------------------------------------------*/
int thresholds[] = {3300, 2000, 1200, 0}; //Can make const?

int convertToRange(int measuredVoltage) { //Convert voltage into levelValue
    int range = 0;

    if (measuredVoltage < 0) { //Invalid measuredVoltage
        return -1;
    }

    for(int i = 1; i < sizeof(thresholds); i++) {
        if (measuredVoltage >= thresholds[i]) {
            range = i;
            break;
        }
    }

    return range;
}

bool started = false;
int counter = 0;
int prevRange = 0;
int currentLevel = 0;
int correctCode[] = {1, 2, 3, 1};
int rangeFrequencies[] = {220, 440, 880, 1760};

#define thresholdStart 1000

bool processIRData(int value) {

    if (started == false) {                           //Deactivate Solenoid when pulled out, start acutal game
        if (value < thresholdStart) {
            if (counter > 2) {
                started = true;
                ESP_ERROR_CHECK(solenoid_set_value(0));
            } else {
                counter++;
            }
        }
    }

    int range = convertToRange(value);

    //play_speaker_sine(rangeFrequencies[range], 100, timer_handle_speaker);
    set_sine_wave(rangeFrequencies[range], 20);

    //TODO: call function to set speaker freq to match range
    if (range == -1) { //invalid value
        return ESP_FAIL;
    }

    if (range == prevRange) {                           //Holding same value
        counter++;
        if (counter >= MAX_COUNT) {                     //Holding for MAX_COUNT amount of IR_SAMPLE_PERIODS (ms)
            if (correctCode[currentLevel] == range) {   //Held correct value
                if (currentLevel == 3) {                //Was last value (4th of 4th LED)
                    currentLevel++;
                    LEDs_set_value(currentLevel);
                    return true;                        //FINISH GAME
                } else {                                //Not last value --> next value
                    currentLevel++;
                    counter = 0;
                }
            } else {                                    //Held Wrong value
                currentLevel = 0;
                counter = 0;
            }
            LEDs_set_value(currentLevel);
        }
    } else {                                            //Value Changed
        prevRange = range;
        counter = 0;
    }
    ESP_LOGI(TAG, "Value: %d\t Range: %d\t Count = %d\t currentLevel = %d\t", value, range, counter, currentLevel);
    return false;
}

