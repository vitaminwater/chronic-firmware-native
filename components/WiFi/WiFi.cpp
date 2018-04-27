/*
 ESP8266WiFi.cpp - WiFi library for esp8266

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 Reworked on 28 Dec 2015 by Markus Sattler

 */
#include "WiFi.h"

#include <iostream>

extern "C" {
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
}


// -----------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------- Debug ------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------------------


/**
 * Output WiFi settings to an object derived from Print interface (like Serial).
 * @param p Print interface
 */
void WiFiClass::printDiag()
{
    const char* modes[] = { "NULL", "STA", "AP", "STA+AP" };

    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);

    uint8_t primaryChan;
    wifi_second_chan_t secondChan;
    esp_wifi_get_channel(&primaryChan, &secondChan);

    bool autoConnect;
    esp_wifi_get_auto_connect(&autoConnect);

    std::cout << "Mode: " << std::endl;
    std::cout << modes[mode] << std::endl;

    std::cout << "Channel: " << std::endl;
    std::cout << primaryChan << std::endl;
    /*
        std::cout << "AP id: " << std::endl;
        std::cout << wifi_station_get_current_ap_id() << std::endl;

        std::cout << "Status: " << std::endl;
        std::cout << wifi_station_get_connect_status() << std::endl;
    */
    std::cout << "Auto connect: " << std::endl;
    std::cout << autoConnect << std::endl;

    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);

    const char* ssid = reinterpret_cast<const char*>(conf.sta.ssid);
    std::cout << "SSID (" << std::endl;
    std::cout << strlen(ssid) << std::endl;
    std::cout << "): " << std::endl;
    std::cout << ssid << std::endl;

    const char* passphrase = reinterpret_cast<const char*>(conf.sta.password);
    std::cout << "Passphrase (" << std::endl;
    std::cout << strlen(passphrase) << std::endl;
    std::cout << "): " << std::endl;
    std::cout << passphrase << std::endl;

    std::cout << "BSSID set: " << std::endl;
    std::cout << conf.sta.bssid_set << std::endl;
}

WiFiClass WiFi;
