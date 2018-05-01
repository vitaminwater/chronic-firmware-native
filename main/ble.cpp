/*
 * =====================================================================================
 *
 *       Filename:  ble.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/24/2018 12:21:25
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <iostream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <BLE2901.h>
#include <BLE2902.h>

#define DEFAULT_NAME "🤖🍁"

BLEServer *pServer;
std::map<std::string, BLEService*> services;

BLEService *get_or_create_service(std::string service);

void init_ble() {
  BLEDevice::init(DEFAULT_NAME);
  pServer = BLEDevice::createServer();
}

void start_advertising() {
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void start_service(std::string service) {
  BLEService *pService = services[service];

  if (pService) {
    pService->start();
  }
}

BLECharacteristic *add_characteristic(std::string service, std::string uuid, std::string name, std::string value) {
  BLEService *pService = get_or_create_service(service);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    uuid,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->addDescriptor(new BLE2901(name));
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue(value);
  return pCharacteristic;
}

BLEService *get_or_create_service(std::string service) {
  BLEService *pService = services[service];

  if (!pService) {
    pService = pServer->createService(service, 50);
    services[service] = pService;
    return pService;
  }
  return pService;
}
