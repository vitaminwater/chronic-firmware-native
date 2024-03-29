/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "conf/version.h"
#include "status_led/status_led.h"
#include "kv/kv.h"
#include "ble/ble.h"
#include "state/state.h"
#include "wifi/wifi.h"
#include "time/time.h"
#include "led/led.h"
#include "timers/timer.h"
#include "timers/manual/manual.h"
#include "timers/onoff/onoff.h"
#include "timers/season/season.h"
#include "mixer/mixer.h"
#include "fans/fans.h"
#include "blower/blower.h"
#include "i2c/i2c.h"
#include "ota/ota.h"
#include "status_led/status_led.h"

void app_main() {
    printf("Welcome to chronic-o-matic firmware %s\n", VERSION);

    init_status_led();
    init_kv();
    init_ble();
    init_state();
    init_wifi();
    init_ota();
    init_led();
    init_time();
    init_fans();
    init_blower();
    init_i2c();

    init_timer();
    init_manual();
    init_onoff();
    init_season();

    init_mixer();

    fflush(stdout);
    while(1) vTaskDelay(10);
}
