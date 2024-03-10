#include "rc522.h"

esp_err_t rc522_power_down();
esp_err_t buzzer_init();
void open_buzzer();
void close_buzzer();
esp_err_t rc522_init(rc522_handle_t* scanner);
esp_err_t myRC522_start(rc522_handle_t* scanner);
esp_err_t detectRestartTag(rc522_handle_t* scanner);