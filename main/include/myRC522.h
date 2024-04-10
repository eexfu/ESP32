#include "driver/gptimer.h"
#include "../component/esp_idf_rc522/include/rc522.h"

esp_err_t rc522_power_down();
esp_err_t solenoid_int();
void open_solenoid();
void close_solenoid();
esp_err_t rc522_init(rc522_handle_t* scanner, gptimer_handle_t* timer_handle);
esp_err_t myRC522_start(rc522_handle_t* scanner);
esp_err_t detectRestartTag(rc522_handle_t* scanner);