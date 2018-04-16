/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>

#include "wifi.h"
#include "kv.h"
#include "led.h"

#define VERSION "v0.1"

void app_main()
{
    printf("Welcome to chronic-o-matic firmware %s\n", VERSION);

    init_kv();
    init_wifi();
    init_time();
    init_led();

    fflush(stdout);
    while(1);
}
