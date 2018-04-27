/*
 * =====================================================================================
 *
 *       Filename:  wifi.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/31/2018 09:45:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef WIFI_H_
#define WIFI_H_

#include <WiFi.h>

void initWifi();
void WifiTask(void *parameter);

#endif
