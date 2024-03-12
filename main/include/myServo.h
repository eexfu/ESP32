#include "driver/mcpwm_prelude.h"
esp_err_t servo_init(int servo_pin, mcpwm_cmpr_handle_t* comparator);
void rotate_servo(int angle, mcpwm_cmpr_handle_t* comparator);