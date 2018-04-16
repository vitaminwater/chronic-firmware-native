/*
 * =====================================================================================
 *
 *       Filename:  light.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/16/2018 14:51:47
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
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include "kv.h"

#define LEDC_LS_CH0_GPIO       (18)
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_LS_CH1_GPIO       (19)
#define LEDC_LS_CH1_CHANNEL    LEDC_CHANNEL_1
#define LEDC_LS_CH2_GPIO       (22)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO       (4)
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3
#define LEDC_LS_CH4_GPIO       (5)
#define LEDC_LS_CH4_CHANNEL    LEDC_CHANNEL_4
#define LEDC_LS_CH5_GPIO       (0)
#define LEDC_LS_CH5_CHANNEL    LEDC_CHANNEL_5

#define LEDC_TEST_CH_NUM       (6)
#define LED_MIN_DUTY         (550)
#define LED_MAX_DUTY         (8191)
#define LEDC_TEST_FADE_TIME    (1000)

typedef struct led_config {
  ledc_channel_t  channel;
  int             gpio;
  int             side;
  int             num;
} led_config_t;

void fade_and_wait_led(ledc_channel_config_t ledc_channel, int duty) {
  ledc_set_fade_with_time(ledc_channel.speed_mode,
      ledc_channel.channel, duty, LEDC_TEST_FADE_TIME);
  ledc_fade_start(ledc_channel.speed_mode,
      ledc_channel.channel, LEDC_FADE_WAIT_DONE);
}

int get_duty_for_time() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo); 

  double day_adv = (timeinfo.tm_hour*60*60*1000 + timeinfo.tm_min*60*1000 + timeinfo.tm_sec*1000) / (24*60*60*1000);
  double year_adv = timeinfo.tm_yday / 365;

  printf("day_adv: %f year_adv: %f\n", day_adv, year_adv);

  return 0;
}

void init_keys(led_config_t config) {
  char power_key[32] = {0};
  sprintf(power_key, "LED_%d_%d_POWER", config.side, config.num);
  if (!hasi(power_key)) {
    seti(power_key, 0);
  }
}

void led_task(void *param) {
  led_config_t config = *((led_config_t *)param);
  init_keys(config);
  ledc_channel_config_t ledc_channel = {
    .channel    = config.channel,
    .duty       = 0,
    .gpio_num   = config.gpio,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_sel  = LEDC_TIMER_1
  };
  ledc_channel_config(&ledc_channel);

  char power_key[32] = {0};
  sprintf(power_key, "LED_%d_%d_POWER", config.side, config.num);
 
  while(1) {
    int power = geti(power_key);
    int duty = get_duty_for_time();

    printf("power: %d duty: %d\n", power, duty);
    vTaskDelay(10*1000 / portTICK_PERIOD_MS);
  }
}

void start_led_task(ledc_channel_t channel, int gpio, int side, int num) {
  led_config_t *conf = (led_config_t *)malloc(sizeof(led_config_t));
  conf->channel = channel;
  conf->gpio = gpio;
  conf->side = side;
  conf->num = num;
  xTaskCreate(led_task, "Led task", 2048, conf, 10, NULL);
}

void init_led() {
  printf("Initializing led task\n");

  ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT,
    .freq_hz = 120,
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_num = LEDC_TIMER_1
  };
  ledc_timer_config(&ledc_timer);

  ledc_fade_func_install(0);

  start_led_task(LEDC_LS_CH0_CHANNEL, LEDC_LS_CH0_GPIO, 0, 0);
  start_led_task(LEDC_LS_CH1_CHANNEL, LEDC_LS_CH1_GPIO, 0, 1);
  start_led_task(LEDC_LS_CH2_CHANNEL, LEDC_LS_CH2_GPIO, 0, 2);

  start_led_task(LEDC_LS_CH3_CHANNEL, LEDC_LS_CH3_GPIO, 1, 0);
  start_led_task(LEDC_LS_CH4_CHANNEL, LEDC_LS_CH4_GPIO, 1, 1);
  start_led_task(LEDC_LS_CH5_CHANNEL, LEDC_LS_CH5_GPIO, 1, 2);
}
