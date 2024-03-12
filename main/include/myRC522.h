#include "rc522.h"

esp_err_t rc522_power_down();
esp_err_t buzzer_init();
esp_err_t solenoid_int();
void open_buzzer();
void close_buzzer();
void open_solenoid();
void close_solenoid();
esp_err_t rc522_init(rc522_handle_t* scanner);
esp_err_t myRC522_start(rc522_handle_t* scanner);
esp_err_t detectRestartTag(rc522_handle_t* scanner);