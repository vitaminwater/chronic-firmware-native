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
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "apps/sntp/sntp.h"

#include "time.h"
#include "kv.h"
#include "ble.h"
#include "wifi.h"

#define TIME_SERVICE "126528f3-7b17-4fd1-ac63-210c9078958e"
#define TIME_UUID "40f9ee4f-e19e-4a8a-aa33-b4aae23b6a9b"
#define START_DATE_MONTH_UUID "829bfbd2-a7e1-4c16-b7e2-4a8fd6261f40"
#define START_DATE_DAY_UUID "547af7e1-6a8c-4fbc-b568-9c3f194cdc1e"
#define DURATION_DAYS_UUID "495600fd-947d-4157-a996-20780ad1d81a"
#define SIMULATION_DURATION_DAYS_UUID "6f01cd48-a405-45e5-99db-0de8b5ca2e7f"
#define STARTED_AT_UUID "1f450234-f101-4f57-ba39-304b053b95a2"

#define TIME "TIME"

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
  defaulti(TIME, 4);
  bleSynci(TIME_SERVICE, TIME_UUID, TIME);
  defaulti(START_DATE_MONTH, 4);
  bleSynci(TIME_SERVICE, START_DATE_MONTH_UUID, START_DATE_MONTH);
  defaulti(START_DATE_DAY, 1);
  bleSynci(TIME_SERVICE, START_DATE_DAY_UUID, START_DATE_DAY);
  defaulti(DURATION_DAYS, 195);
  bleSynci(TIME_SERVICE, DURATION_DAYS_UUID, DURATION_DAYS);
  defaulti(SIMULATION_DURATION_DAYS, 75);
  bleSynci(TIME_SERVICE, SIMULATION_DURATION_DAYS_UUID, SIMULATION_DURATION_DAYS);
  defaulti(STARTED_AT, 0);
  bleSynci(TIME_SERVICE, STARTED_AT_UUID, STARTED_AT);
  start_service(TIME_SERVICE);

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

  struct tm tm_simulated;
  memset(&tm_simulated, 0, sizeof(tm_simulated));
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
  if (hasi(TIME)) {
    time_t now = (time_t)geti(TIME);
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
    seti(TIME, (int)now);

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
  const char *NTP_SERVER = "pool.ntp.org";
  printf("Initializing SNTP\n");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char *)NTP_SERVER);
  sntp_init();
}
