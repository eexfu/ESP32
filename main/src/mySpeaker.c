#include "mySpeaker.h"

//copy from sda_dac example of Expressif
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/sdm.h"
#include "driver/gptimer.h"

#define MHZ                             (1000000)
#define CONST_PI                        (3.1416f)           // Constant of PI, used for calculating the sine wave
#define SIGMA_DELTA_GPIO_NUM    (3)                 // Select GPIO_NUM_3 as the sigma-delta output pin
#define OVER_SAMPLE_RATE        (10 * MHZ)          // 10 MHz over sample rate
#define TIMER_RESOLUTION        (1  * MHZ)          // 1 MHz timer counting resolution
#define CALLBACK_INTERVAL_US    (100)               // 100 us interval of each timer callback
#define ALARM_COUNT             (CALLBACK_INTERVAL_US * (TIMER_RESOLUTION / MHZ))

ESP_STATIC_ASSERT(CALLBACK_INTERVAL_US >= 7, "Timer callback interval is too short");

static const char *TAG = "SDM";
static int8_t* sine_wave;
static int number_of_points;
static int audio_samples_count = 20000;
static volatile bool playbackFinished = false;
static volatile bool playing = false;

static bool IRAM_ATTR timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *data, void *user_ctx){
    static uint32_t cnt = 0;
    sdm_channel_handle_t sdm_chan = (sdm_channel_handle_t)user_ctx;
    /* Set the pulse density */
    sdm_channel_set_pulse_density(sdm_chan, sine_wave[cnt++]);
    /* Loop the sine wave data buffer */
    if (cnt >= number_of_points) {
        cnt = 0;
        if(number_of_points == audio_samples_count){
            playbackFinished = true;
            gptimer_stop(timer);
        }
    }
    return false;
}

static gptimer_handle_t gptimer_init(void* args)
{
    /* Allocate GPTimer handle */
    gptimer_handle_t timer_handle;
    gptimer_config_t timer_cfg = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = TIMER_RESOLUTION,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_cfg, &timer_handle));
    ESP_LOGI(TAG, "Timer allocated with resolution %d Hz", TIMER_RESOLUTION);

    /* Set the timer alarm configuration */
    gptimer_alarm_config_t alarm_cfg = {
            .alarm_count = ALARM_COUNT,
            .reload_count = 0,
            .flags.auto_reload_on_alarm = true,
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer_handle, &alarm_cfg));

    /* Register the alarm callback */
    gptimer_event_callbacks_t cbs = {
            .on_alarm = timer_callback,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer_handle, &cbs, args));
    ESP_LOGI(TAG, "Timer callback registered, interval %d us", CALLBACK_INTERVAL_US);

    /* Clear the timer raw count value, make sure it'll count from 0 */
    ESP_ERROR_CHECK(gptimer_set_raw_count(timer_handle, 0));
    /* Enable the timer */
    ESP_ERROR_CHECK(gptimer_enable(timer_handle));

    ESP_LOGI(TAG, "Timer enabled");

    return timer_handle;
}

static sdm_channel_handle_t sam_init(void)
{
    /* Allocate sdm channel handle */
    sdm_channel_handle_t sdm_chan = NULL;
    sdm_config_t config = {
            .clk_src = SDM_CLK_SRC_DEFAULT,
            .gpio_num = SIGMA_DELTA_GPIO_NUM,
            .sample_rate_hz = OVER_SAMPLE_RATE,
    };
    ESP_ERROR_CHECK(sdm_new_channel(&config, &sdm_chan));
    /* Enable the sdm channel */
    ESP_ERROR_CHECK(sdm_channel_enable(sdm_chan));

    ESP_LOGI(TAG, "Sigma-delta output is attached to GPIO %d", SIGMA_DELTA_GPIO_NUM);

    return sdm_chan;
}

esp_err_t speaker_init(gptimer_handle_t* timer_handle){
    /* Initialize sigma-delta modulation on the specific GPIO */
    sdm_channel_handle_t sdm_chan = sam_init();
    /* Initialize GPTimer and register the timer alarm callback */
    *timer_handle = gptimer_init(sdm_chan);
    return ESP_OK;
}

void timer(void* param){
    gptimer_handle_t timer_handle = *(gptimer_handle_t*)param;
    gptimer_start(timer_handle);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gptimer_stop(timer_handle);
    free(sine_wave);
    vTaskDelete(NULL);
}

void timer_audio(void* param){
    gptimer_handle_t timer_handle = *(gptimer_handle_t*)param;
    gptimer_start(timer_handle);
    while (!playbackFinished){
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    playbackFinished = false;
    free(sine_wave);
    playing = false;
    vTaskDelete(NULL);
}

esp_err_t play_speaker_sine(int frequency, int amplitude, gptimer_handle_t* timer_handle){
    esp_err_t ret;

    number_of_points = MHZ / (frequency * CALLBACK_INTERVAL_US);  //freq = 1000Hz -> 10
    if(number_of_points <= 1) printf("Sine wave frequency is too high");

    int8_t* audio_samples = (int8_t*)malloc(number_of_points * sizeof(int8_t));
    /* Assign sine wave data */
    for (int i = 0; i < number_of_points; i++) {
        audio_samples[i] = (int8_t)((sin(2 * (float)i * CONST_PI / number_of_points)) * amplitude);
        printf("%d\n",audio_samples[i]);
    }
    sine_wave = audio_samples;
    /* Start the GPTimer */
    ESP_LOGI(TAG, "Speaker start");
    ret = xTaskCreate((TaskFunction_t) &timer, "TIMER", 1024 * 3, (void *)timer_handle, 2, NULL);
    return ret;
}

esp_err_t play_sine_start(int frequency, int amplitude, gptimer_handle_t* timer_handle){
    esp_err_t ret;

    number_of_points = MHZ / (frequency * CALLBACK_INTERVAL_US);  //freq = 1000Hz -> 10
    if(number_of_points <= 1) printf("Sine wave frequency is too high");

    int8_t* audio_samples = (int8_t*)malloc(number_of_points * sizeof(int8_t));
    /* Assign sine wave data */
    for (int i = 0; i < number_of_points; i++) {
        audio_samples[i] = (int8_t)((sin(2 * (float)i * CONST_PI / number_of_points)) * amplitude);
        printf("%d\n",audio_samples[i]);
    }
    sine_wave = audio_samples;
    /* Start the GPTimer */
    ESP_LOGI(TAG, "Speaker start");
    ret = gptimer_start(*timer_handle);
    return ret;
}

esp_err_t play_sine_stop(gptimer_handle_t* timer_handle){
    esp_err_t ret;

    ret = gptimer_stop(*timer_handle);
    free(sine_wave);

    return ret;
}

void set_sine_wave(int frequency, int amplitude){
    number_of_points = MHZ / (frequency * CALLBACK_INTERVAL_US);  //freq = 1000Hz -> 10
    if(number_of_points <= 1) printf("Sine wave frequency is too high");

    int8_t* audio_samples = (int8_t*)malloc(number_of_points * sizeof(int8_t));
    /* Assign sine wave data */
    for (int i = 0; i < number_of_points; i++) {
        audio_samples[i] = (int8_t)((sin(2 * (float)i * CONST_PI / number_of_points)) * amplitude);
        printf("%d\n",audio_samples[i]);
    }
    sine_wave = audio_samples;
}

esp_err_t play_speaker_audio(const char* file_name, gptimer_handle_t* timer_handle) {
    if(playing){
        return ESP_FAIL;
    } else{
        playing = true;
    }

    FILE* file;
    int8_t* audio_samples;
    esp_err_t ret;

    // open file
    file = fopen(file_name, "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file");
        return ESP_FAIL;
    }

    // allocate memory
    audio_samples = (int8_t*)malloc(audio_samples_count * sizeof(int8_t));
    if (!audio_samples) {
        ESP_LOGE(TAG, "Failed to allocate memory for audio samples");
        fclose(file);
        return ESP_FAIL;
    }

    // read data
    int tmp;
    for(int i = 0; i < audio_samples_count; i++) {
        if (fscanf(file, "%d", &tmp) != 1) {
            ESP_LOGE(TAG, "Failed to read audio samples");
            free(audio_samples);
            fclose(file);
            return ESP_FAIL;
        }
        audio_samples[i] = (int8_t)tmp;
    }

    fclose(file);

    sine_wave = audio_samples;
    number_of_points = audio_samples_count;

    /* Start the GPTimer */
    ESP_LOGI(TAG, "Speaker start");
    ret = xTaskCreate((TaskFunction_t) timer_audio, "TIMER", 1024 * 3, (void*)timer_handle, 2, NULL);
    return ret;
}