/*
 * =====================================================================================
 *
 *       Filename:  kv.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/16/2018 15:47:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "kv.h"

nvs_handle open_handle() {
  nvs_handle kv_handle;
  esp_err_t err = nvs_open("kv_store", NVS_READWRITE, &kv_handle);
  if (err != ESP_OK) {
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  return kv_handle;
}

void init_kv() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK( err );

  open_handle();
}

bool hasi(const char *key) {
  int32_t value;
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_get_i32(kv_handle, key, &value);
  nvs_close(kv_handle);
  return err == ESP_OK;
}

int geti(const char *key) {
  nvs_handle kv_handle = open_handle();
  int32_t value;
  esp_err_t err = nvs_get_i32(kv_handle, key, &value);
  ESP_ERROR_CHECK(err);
  nvs_close(kv_handle);
  return (int)value;
}

void seti(const char *key, int value) {
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_set_i32(kv_handle, key, (int32_t)value);
  ESP_ERROR_CHECK(err);
  nvs_commit(kv_handle);
  nvs_close(kv_handle);
}

void defaulti(const char *key, int value) {
  if (!hasi(key)) {
    seti(key, value);
  }
}

bool hasstr(const char *key) {
  nvs_handle kv_handle = open_handle();
  size_t length;
  esp_err_t err = nvs_get_str(kv_handle, key, NULL, &length);
  nvs_close(kv_handle);
  return err == ESP_OK;
}

void getstr(const char *key, char *value, size_t length) {
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_get_str(kv_handle, key, value, &length);
  ESP_ERROR_CHECK(err);
  nvs_close(kv_handle);
}

void setstr(const char *key, const char *value) {
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_set_str(kv_handle, key, value);
  ESP_ERROR_CHECK(err);
  nvs_commit(kv_handle);
  nvs_close(kv_handle);
}

void defaultstr(const char *key, const char *value) {
  if (!hasstr(key)) {
    setstr(key, value);
  }
}
