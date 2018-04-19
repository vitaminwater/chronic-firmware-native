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

#define START_DATE_MONTH "SIM_S_M"
#define START_DATE_DAY "SIM_S_D"
#define DURATION_DAYS "DUR_D"
#define SIMULATION_DURATION_DAYS "SIM_DUR_D"
#define STARTED_AT "ST_AT"

void time_task(void *param);
void ntp_task(void *param);
static void setup(void);
void print_time(time_t t);
void print_timeinfo(struct tm timeinfo);

void init_time() {
  defaulti(START_DATE_MONTH, 4);
  defaulti(START_DATE_DAY, 1);
  defaulti(DURATION_DAYS, 195);
  defaulti(SIMULATION_DURATION_DAYS, 75);
  defaulti(STARTED_AT, 0);

  seti(STARTED_AT, (int)1518912000); // 02
  //seti(STARTED_AT, (int)1521331200); // 03

  xTaskCreate(time_task, "Time Task", 2048, NULL, 10, NULL);
}

time_t get_box_time() {
  int start_date_month = geti(START_DATE_MONTH);
  int start_date_day = geti(START_DATE_DAY);
  int duration_days = geti(DURATION_DAYS);
  int simulation_duration_days = geti(SIMULATION_DURATION_DAYS);
  int started_at = geti(STARTED_AT);

  time_t now;
  struct tm tm_now;
  time(&now);
  localtime_r(&now, &tm_now); 

  time_t started_at_t = (time_t)started_at;
  struct tm tm_started_at;
  localtime_r(&started_at_t, &tm_started_at);

  double duration = (double)((int)now - (int)started_at_t) / (double)(24 * 60 * 60);
  double adv = (double)duration / (double)simulation_duration_days;

  struct tm tm_simulated = {0};
  tm_simulated.tm_year = tm_started_at.tm_year;
  tm_simulated.tm_mon = start_date_month;
  tm_simulated.tm_mday = start_date_day;
  tm_simulated.tm_hour = tm_now.tm_hour;
  tm_simulated.tm_min = tm_now.tm_min;
  tm_simulated.tm_sec = tm_now.tm_sec;

  time_t simulated_time = mktime(&tm_simulated);
  int seconds_to_adv = adv * duration_days * 24 * 60 * 60;
  simulated_time += seconds_to_adv - (seconds_to_adv % (24 * 60 * 60));
  localtime_r(&simulated_time, &tm_simulated);

  return simulated_time;
}

void time_task(void *param) {
  if (hasi("TIME")) {
    time_t now = (time_t)geti("TIME");
    struct timeval tv = { .tv_sec = now, .tv_usec = 0 };
    settimeofday(&tv, NULL);
    print_time(now);
    xTaskCreate(ntp_task, "NTP task", 2048, NULL, 10, NULL);
  } else {
    wait_connected();
    setup();
  }
  while(true) {
    time_t now;
    time(&now);
    print_time(now);
    seti("TIME", (int)now);

    printf("Simulated time: ");
    print_time(get_box_time());

    vTaskDelay(30 * 1000 / portTICK_PERIOD_MS);
  }
}

void print_time(time_t t) {
  struct tm timeinfo;
  localtime_r(&t, &timeinfo); 

  print_timeinfo(timeinfo);
}

void print_timeinfo(struct tm timeinfo) {
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
