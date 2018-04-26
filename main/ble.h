/*
 * =====================================================================================
 *
 *       Filename:  ble.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/24/2018 12:20:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef BLE_H_
#define BLE_H_

#include <BLECharacteristic.h>

void init_ble();
void start_service(std::string service);
void start_advertising();

BLECharacteristic *add_characteristic(std::string service, std::string uuid, std::string name, std::string value);

#endif
