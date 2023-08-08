// Copyright (C) 2018 Alex Goris
// This file is part of FlyballETS-Software
// FlyballETS-Software is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>

#ifndef _WIFIMANAGER_H
#define _WIFIMANAGER_H

#include "Arduino.h"
#include <ArduinoOTA.h>
#include <WiFi.h>

#define WIFI_CHECK_INTERVAL 5000

class WifiManagerClass
{
public:
    void SetupWiFi();
    void WiFiLoop();
    void WiFiEvent(arduino_event_id_t event);
    void ToggleWifi();
    
    // Global variables
    bool bCheckWsClinetStatus = false; // flag to check if WS client should be disconnected
    IPAddress ipTocheck;               // IP address of disconnected WiFi user

private:
    void mdnsServerSetup();
    IPAddress _IPGateway;
    IPAddress _IPSubnet;
    String _strAPName;
    String _strSTAName;
    uint16_t uiLastProgress = 0; // last % OTA progress value
    unsigned long ulLastWifiCheck = 0;
};

extern WifiManagerClass WifiManager;

#endif