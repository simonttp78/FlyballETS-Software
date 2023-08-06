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

#include "WifiManager.h"
#include <BlueNodeHandler.h>
#include <SettingsManager.h>
#include "enums.h"
#include "WiFi.h"

void WifiManager::SetupWiFi()
{
    WiFi.onEvent(std::bind(&WifiManager::WiFiEvent, this, std::placeholders::_1));
    /*WiFi.onEvent(
    [this](WiFiEvent_t event, system_event_info_t info) {
        this->WiFiEvent(event);
    });*/
    // WiFi.onEvent(WiFiEvent);
    uint8_t iOpMode = SettingsManager.getSetting("OperationMode").toInt();
    _strAPName = SettingsManager.getSetting("APName");
    String strAPPass = SettingsManager.getSetting("APPass");
    log_i("Starting in mode %i", iOpMode);
    log_i("Name: %s, pass: %s\r\n", _strAPName.c_str(), strAPPass.c_str());
    if (iOpMode == SystemModes::RED || iOpMode == SystemModes::SINGLE)
    {
        _IPGateway = IPAddress(192, 168, 20, 1);
        _IPSubnet = IPAddress(255, 255, 255, 0);
        if (!WiFi.mode(WIFI_MODE_AP) ||
            !WiFi.softAPConfig(_IPGateway, _IPGateway, _IPSubnet) ||
            !WiFi.softAP(_strAPName.c_str(), strAPPass.c_str()))
        {
            log_w("[WiFi]: Error initializing softAP with name %s!", _strAPName.c_str());
        }
    }
    else if (iOpMode == SystemModes::BLUE)
    {
        _strSTAName = _strAPName;
        _strAPName += "_SLV";
        _IPGateway = IPAddress(192, 168, 4, 1);
        _IPSubnet = IPAddress(255, 255, 255, 0);
        if (!WiFi.mode(WIFI_MODE_APSTA) || !WiFi.softAP(_strAPName.c_str(), strAPPass.c_str()))
        {
            log_w("[WiFi]: Error initializing softAP with name %s!", _strAPName.c_str());
        }
        WiFi.begin(_strSTAName.c_str(), strAPPass.c_str());
    }
    else
        log_e("[WiFi]: Got unknown mode, no idea how I should start...");
}

void WifiManager::WiFiLoop()
{
    if (millis() - ulLastWifiCheck > WIFI_CHECK_INTERVAL)
    {
        ulLastWifiCheck = millis();
        log_v("Wifi Status: %u, Wifi.localIP: %S, SoftAPIP: %s, SoftAPStationNum: %i",
              WiFi.status(),
              WiFi.localIP().toString().c_str(),
              WiFi.softAPIP().toString().c_str(),
              WiFi.softAPgetStationNum());
    }
}

void WifiManager::WiFiEvent(arduino_event_id_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_START:
        log_i("Trying to configure IP: %s, SM: %s", _IPGateway.toString().c_str(), _IPSubnet.toString().c_str());
        if (!WiFi.softAPConfig(_IPGateway, _IPGateway, _IPSubnet))
        {
            log_e("[WiFi]: AP Config failed!");
        }
        else
        {
            log_i("[WiFi]: AP Started with name %s, IP: %s", _strAPName.c_str(), WiFi.softAPIP().toString().c_str());
        }
        if (WiFi.softAPIP() != _IPGateway)
        {
            log_e("I am not running on the correct IP (%s instead of %s), rebooting!", WiFi.softAPIP().toString().c_str(), _IPGateway.toString().c_str());
            ESP.restart();
        }
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        log_i("[WiFi]: AP Stopped");
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        log_i("[WiFi]: STA got IP %s", WiFi.localIP().toString().c_str());
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    {
        BlueNodeHandler.resetConnection();
        log_i("Disconnected from AP (event %i)", ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        String _strAPName = SettingsManager.getSetting("APName");
        String strAPPass = SettingsManager.getSetting("APPass");
        WiFi.begin(_strAPName.c_str(), strAPPass.c_str());
        log_w("[WiFi]: Connecting to AP...");
        break;
    }

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        log_i("Connected to AP %s (event %i)", WiFi.SSID().c_str(), ARDUINO_EVENT_WIFI_STA_CONNECTED);
        break;

    default:
        log_d("Wifi event %i\r\n", event);
        break;
    }
}