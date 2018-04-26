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
#include <string.h>
#include <iostream>

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

bool hasi(std::string key) {
  int32_t value;
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_get_i32(kv_handle, (const char *)key.c_str(), &value);
  nvs_close(kv_handle);
  return err == ESP_OK;
}

int geti(std::string key) {
  nvs_handle kv_handle = open_handle();
  int32_t value;
  esp_err_t err = nvs_get_i32(kv_handle, (const char *)key.c_str(), &value);
  ESP_ERROR_CHECK(err);
  nvs_close(kv_handle);
  return (int)value;
}

void seti(std::string key, int value) {
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_set_i32(kv_handle, (const char *)key.c_str(), (int32_t)value);
  ESP_ERROR_CHECK(err);
  nvs_commit(kv_handle);
  nvs_close(kv_handle);
}

void defaulti(std::string key, int value) {
  if (!hasi(key)) {
    seti(key, value);
  }
}

bool hasstr(std::string key) {
  nvs_handle kv_handle = open_handle();
  size_t length;
  esp_err_t err = nvs_get_str(kv_handle, (const char *)key.c_str(), NULL, &length);
  nvs_close(kv_handle);
  return err == ESP_OK;
}

std::string getstr(std::string key) {
  size_t length;
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_get_str(kv_handle, (const char *)key.c_str(), NULL, &length);
  ESP_ERROR_CHECK(err);

  char *value = (char *)malloc(length + 1);
  memset(value, 0, length + 1);
  err = nvs_get_str(kv_handle, (const char *)key.c_str(), value, &length);
  ESP_ERROR_CHECK(err);

  nvs_close(kv_handle);

  std::string res = std::string(value);
  free(value);
  return res;
}

void setstr(std::string key, std::string value) {
  nvs_handle kv_handle = open_handle();
  esp_err_t err = nvs_set_str(kv_handle, (const char *)key.c_str(), value.c_str());
  ESP_ERROR_CHECK(err);
  nvs_commit(kv_handle);
  nvs_close(kv_handle);
}

void defaultstr(std::string key, std::string value) {
  if (!hasstr(key)) {
    setstr(key, value);
  }
}
