/*
 * =====================================================================================
 *
 *       Filename:  time.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/16/2018 19:26:47
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "apps/sntp/sntp.h"

#include "time.h"
#include "kv.h"
#include "wifi.h"

void time_task(void *param);
void ntp_task(void *param);
static void setup(void);

void init_time() {
  xTaskCreate(time_task, "Time Task", 2048, NULL, 10, NULL);
}

void time_task(void *param) {
  if (hasi("time")) {
    time_t now = (time_t)geti("time");
    struct timeval tv = { .tv_sec = now, .tv_usec = 0 };
    settimeofday(&tv, NULL);
    xTaskCreate(ntp_task, "NTP task", 2048, NULL, 10, NULL);
  } else {
    wait_connected();
    setup();
  }
  while(true) {
    print_now();

    seti("time", (int)now);
    vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
  }
}

void print_now() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo); 

  char buffer[26] = {0};
  strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", &timeinfo);
  printf("%s\n", buffer);
}

void ntp_task(void *param) {
  wait_connected();
  setup();
  vTaskDelete(NULL);
}

static void setup(void) {
  printf("Initializing SNTP\n");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_init();
}
