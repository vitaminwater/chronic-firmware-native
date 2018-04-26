/*
 * =====================================================================================
 *
 *       Filename:  kv_ble.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/24/2018 16:53:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <string>

#include "ble.h"
#include "kv.h"

#include <BLECharacteristic.h>

#include <sstream>
 
template <typename T>
std::string to_string(const T& thing)
{
  std::stringstream ss;
  ss << thing;
  return ss.str();
}

long stoi(std::string str)
{
  const char *s = str.c_str();
  long i;
  i = 0;
  while(*s >= '0' && *s <= '9')
  {
    i = i * 10 + (*s - '0');
    s++;
  }
  return i;
}

class IntCallbacks: public BLECharacteristicCallbacks {

  public:

  IntCallbacks(std::string k);
  void onRead(BLECharacteristic* pCharacteristic);
  void onWrite(BLECharacteristic* pCharacteristic);

  private:

  std::string key;
};

IntCallbacks::IntCallbacks(std::string k) : key(k) {}
void IntCallbacks::onRead(BLECharacteristic* pCharacteristic) {}

void IntCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
  seti(key, stoi(pCharacteristic->getValue()));
}

class StrCallbacks: public BLECharacteristicCallbacks {

  public:

  StrCallbacks(std::string k);
  void onRead(BLECharacteristic* pCharacteristic);
  void onWrite(BLECharacteristic* pCharacteristic);

  private:

  std::string key;
};

StrCallbacks::StrCallbacks(std::string k) : key(k) {}

void StrCallbacks::onRead(BLECharacteristic* pCharacteristic) {}

void StrCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
  setstr(key, pCharacteristic->getValue());
}

void bleSynci(std::string service, std::string uuid, std::string key) {
  BLECharacteristic *pCharacteristic = add_characteristic(service, uuid, key, to_string(geti(key)));
  pCharacteristic->setCallbacks(new IntCallbacks(key));
}

void bleSyncstr(std::string service, std::string uuid, std::string key) {
  BLECharacteristic *pCharacteristic = add_characteristic(service, uuid, key, getstr(key));
  pCharacteristic->setCallbacks(new StrCallbacks(key));
}
