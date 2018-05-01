/*
 * =====================================================================================
 *
 *       Filename:  wifi.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/31/2018 09:31:28
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "kv.h"
#include "ble.h"
#include "wifi.h"

#define WIFI_CMD_SCAN     1
#define WIFI_CMD_CONNECT  2

QueueHandle_t cmdQ;

#define WIFI_SSID_PREF      "SSID"
#define WIFI_PASSWORD_PREF  "PASSWORD"

int previous_status;
#define WIFI_STATUS_NO_SSID       "WIFI_STATUS_NO_SSID"
#define WIFI_STATUS_SCANNING      "WIFI_STATUS_SCANNING"
#define WIFI_STATUS_CONNECTING    "WIFI_STATUS_CONNECTING"
#define WIFI_STATUS_CONNECTED     "WIFI_STATUS_CONNECTED"
#define WIFI_STATUS_DISCONNECTED  "WIFI_STATUS_DISCONNECTED"
#define WIFI_STATUS_ERROR         "WIFI_STATUS_ERROR"

#define WIFI_SERVICE_UUID                   "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define WIFI_STATUS_CHARACTERISTIC_UUID     "ff05b1e7-669b-4678-8882-467f029f5165"
#define SCAN_WIFI_CHARACTERISTIC_UUID       "718226eb-2bc4-4244-b191-02c34d31cde3"
#define FOUND_WIFI_CHARACTERISTIC_UUID      "d764b280-a943-474a-ad58-2ca6d28488ea"
#define WIFI_SSID_CHARACTERISTIC_UUID       "8ca48df9-f388-4a41-9ae1-36bf4e2a8517"
#define WIFI_PASSWORD_CHARACTERISTIC_UUID   "2af8b831-3927-49e6-b1bc-8e8aab7434e3"

BLEService *pService;
BLECharacteristic *wifiStatusCharacteristic;
BLECharacteristic *scanWifiCharacteristic;
BLECharacteristic *foundWifiCharacteristic;
BLECharacteristic *wifiSSIDCharacteristic;
BLECharacteristic *wifiPasswordCharacteristic;

class ScanCallbacks: public BLECharacteristicCallbacks {

  public:

  ScanCallbacks();
  void onRead(BLECharacteristic* pCharacteristic);
  void onWrite(BLECharacteristic* pCharacteristic);

  private:
};

ScanCallbacks::ScanCallbacks() {}
void ScanCallbacks::onRead(BLECharacteristic* pCharacteristic) {}

void ScanCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
  Serial.println(pCharacteristic->getValue().c_str());
  if (pCharacteristic->getValue() == "ON") { 
    char cmd = WIFI_CMD_SCAN;
    xQueueSend(cmdQ, &cmd, ( TickType_t ) 10);   
  }
  pCharacteristic->notify();
}

class SSIDCallbacks: public BLECharacteristicCallbacks {

  public:

  SSIDCallbacks();
  void onRead(BLECharacteristic* pCharacteristic);
  void onWrite(BLECharacteristic* pCharacteristic);

  private:
};

SSIDCallbacks::SSIDCallbacks() {}
void SSIDCallbacks::onRead(BLECharacteristic* pCharacteristic) {}

void SSIDCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
  setString(WIFI_SSID_PREF, pCharacteristic->getValue().c_str());
}

class PasswordCallbacks: public BLECharacteristicCallbacks {

  public:

  PasswordCallbacks();
  void onRead(BLECharacteristic* pCharacteristic);
  void onWrite(BLECharacteristic* pCharacteristic);

  private:
};

PasswordCallbacks::PasswordCallbacks() {}
void PasswordCallbacks::onRead(BLECharacteristic* pCharacteristic) {}

void PasswordCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
  setString(WIFI_PASSWORD_PREF, pCharacteristic->getValue().c_str());

  if (wifiSSIDCharacteristic->getValue() != "" && wifiPasswordCharacteristic->getValue() != "") {
    char cmd = WIFI_CMD_CONNECT;
    xQueueSend(cmdQ, &cmd, ( TickType_t ) 10);   
  }
}

void init_wifi() {
  bool hasSSIDPass = getstr(WIFI_SSID_PREF).length() && getstr(WIFI_PASSWORD_PREF).length();

  previous_status = WiFi.status();
  if (hasSSIDPass) {
    Serial.println("SSID available");
    WiFi.begin(getstr(WIFI_SSID_PREF).c_str(), getstr(WIFI_PASSWORD_PREF).c_str()); 
  }

  cmdQ = xQueueCreate(10, sizeof(char));
  if(cmdQ == NULL){
    Serial.println("Error creating the queue");
  }
  pService = newService(WIFI_SERVICE_UUID);

  wifiStatusCharacteristic = pService->createCharacteristic(
      WIFI_STATUS_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
      );
  wifiStatusCharacteristic->addDescriptor(new BLE2902());
  wifiStatusCharacteristic->setValue(hasSSIDPass ? WIFI_STATUS_CONNECTING : WIFI_STATUS_NO_SSID);

  scanWifiCharacteristic = pService->createCharacteristic(
      SCAN_WIFI_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
      );
  scanWifiCharacteristic->addDescriptor(new BLE2902());
  scanWifiCharacteristic->setValue("OFF");
  scanWifiCharacteristic->setCallbacks(new ScanCallbacks());

  foundWifiCharacteristic = pService->createCharacteristic(
      FOUND_WIFI_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
      );
  foundWifiCharacteristic->addDescriptor(new BLE2902());
  foundWifiCharacteristic->setValue("NOT_WIFI");

  wifiSSIDCharacteristic = pService->createCharacteristic(
      WIFI_SSID_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
      );
  wifiSSIDCharacteristic->setValue(getstr(WIFI_SSID_PREF).c_str());
  wifiSSIDCharacteristic->setCallbacks(new SSIDCallbacks());

  wifiPasswordCharacteristic = pService->createCharacteristic(
      WIFI_PASSWORD_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_WRITE
      );
  wifiPasswordCharacteristic->setValue("");
  wifiPasswordCharacteristic->setCallbacks(new PasswordCallbacks());
}

void scanTask(void *parameter) {
  Serial.println("WIFI_CMD_SCAN");
  std::string currentStatus = wifiStatusCharacteristic->getValue();
  wifiStatusCharacteristic->setValue(WIFI_STATUS_SCANNING);
  wifiStatusCharacteristic->notify();
  int n = WiFi.scanNetworks();
  Serial.println(n);
  for (int i = 0; i < n; ++i) {
    String name = WiFi.SSID(i) +
      "::" +
      String(WiFi.RSSI(i)) +
      "::" +
      (WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? " " : "*");
    foundWifiCharacteristic->setValue(name.c_str());
    foundWifiCharacteristic->notify();
    Serial.println(name);
    delay(20);
  }
  Serial.println("DONE");
  wifiStatusCharacteristic->setValue(currentStatus);
  wifiStatusCharacteristic->notify();
  vTaskDelete(NULL);
}

void WifiTask(void *parameter) {
  Serial.println("Started Wifi task");
  pService->start();

  while(true) {
    char cmd = 0;

    if (xQueueReceive(cmdQ, &cmd, 100)) { 
      switch (cmd) {
        case WIFI_CMD_SCAN:
          xTaskCreate(scanTask, "ScanTask", 10000, NULL, 1, NULL);
          break;
        case WIFI_CMD_CONNECT:
          Serial.println("WIFI_CMD_CONNECT");
          wifiStatusCharacteristic->setValue(WIFI_STATUS_CONNECTING);
          wifiStatusCharacteristic->notify();
          WiFi.begin(getstr(WIFI_SSID_PREF).c_str(), getstr(WIFI_PASSWORD_PREF).c_str()); 
          break;
      }
    }
    if (WiFi.status() != previous_status) {
      if (WiFi.status() == WL_CONNECTED) {
        wifiStatusCharacteristic->setValue(WIFI_STATUS_CONNECTED);
      } else if (WiFi.status() == WL_CONNECT_FAILED && WiFi.status() == WL_CONNECTION_LOST) {
        wifiStatusCharacteristic->setValue(WIFI_STATUS_DISCONNECTED);
      } else if (WiFi.status() == WL_DISCONNECTED) {
        wifiStatusCharacteristic->setValue(WIFI_STATUS_DISCONNECTED);
      } else if (WiFi.status() == WL_NO_SSID_AVAIL) {
        wifiStatusCharacteristic->setValue(WIFI_STATUS_NO_SSID);
      }
      wifiStatusCharacteristic->notify();
      previous_status = WiFi.status();
      Serial.print("Wifi status changed ");
      Serial.println(WiFi.status());
    }
  }
}
