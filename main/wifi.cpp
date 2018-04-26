/*
 * =====================================================================================
 *
 *       Filename:  wifi.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/16/2018 19:24:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "esp_sleep.h"
#include "nvs_flash.h"

#include "wifi.h"

#define DEFAULT_SSID "FastAndSeriousSoft"
#define DEFAULT_PASS "42FastAndSeriousSoft42"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static void setup(void);
static esp_err_t event_handler(void *ctx, system_event_t *event);

void init_wifi() {
  setup();
}

void wait_connected() {
  xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
      false, true, portMAX_DELAY);
}

static void setup(void)
{
  tcpip_adapter_init();
  wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
  wifi_config_t wifi_config;
  strcpy((char *)wifi_config.sta.ssid, DEFAULT_SSID);
  strcpy((char *)wifi_config.sta.password, DEFAULT_PASS);
  wifi_config.sta.bssid_set = false;
  printf("Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
  ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
  ESP_ERROR_CHECK( esp_wifi_start() );
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
  switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
      printf("SYSTEM_EVENT_STA_START\n");
      esp_wifi_connect();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      printf("SYSTEM_EVENT_STA_GOT_IP\n");
      xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      printf("SYSTEM_EVENT_STA_DISCONNECTED\n");
      /* This is a workaround as ESP32 WiFi libs don't currently
         auto-reassociate. */
      esp_wifi_connect();
      xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
      break;
    default:
      break;
  }
  return ESP_OK;
}
