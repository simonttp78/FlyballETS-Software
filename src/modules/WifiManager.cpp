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
#include "BlueNodeHandler.h"
#include "SettingsManager.h"
#include "LCDController.h"
#include <ESPmDNS.h>
#include "enums.h"

void WifiManagerClass::SetupWiFi()
{
    WiFi.onEvent(std::bind(&WifiManagerClass::WiFiEvent, this, std::placeholders::_1));
    uint8_t iOpMode = SettingsManager.getSetting("OperationMode").toInt();
    _strAPName = SettingsManager.getSetting("APName");
    _strAPPass = SettingsManager.getSetting("APPass");
    log_i("Starting ETS mode %i", iOpMode);
    log_i("Reference SSID: %s, pass: %s\r\n", _strAPName.c_str(), _strAPPass.c_str());
    if (iOpMode == SystemModes::RED || iOpMode == SystemModes::SINGLE)
    {
        _IPGateway = IPAddress(192, 168, 20, 1);
        _IPSubnet = IPAddress(255, 255, 255, 0);
        if (!WiFi.mode(WIFI_MODE_AP) ||
            !WiFi.softAP(_strAPName.c_str(), _strAPPass.c_str()) ||
            !WiFi.softAPConfig(_IPGateway, _IPGateway, _IPSubnet))
        {
            log_w("[WiFi]: Error initializing softAP with name %s!", _strAPName.c_str());
        }

        // OTA setup
        String _strAPPass = SettingsManager.getSetting("APPass");
        ArduinoOTA.setPassword(_strAPPass.c_str());
        ArduinoOTA.setPort(3232);
        ArduinoOTA.onStart([](){
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "Firmware";
            else // U_SPIFFS
                type = "Filesystem";
            Serial.println("\n" + type + " update initiated.");
            LCDController.FirmwareUpdateInit(); });
        ArduinoOTA.onEnd([](){ 
            Serial.println("\nUpdate completed.\r\n");
            LCDController.FirmwareUpdateSuccess(); });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total){
            uint16_t iProgressPercentage = (progress / (total / 100));
            if (WifiManager.uiLastProgress != iProgressPercentage)
            {
                Serial.printf("Progress: %u%%\r", iProgressPercentage);
                String sProgressPercentage = String(iProgressPercentage);
                while (sProgressPercentage.length() < 3)
                    sProgressPercentage = " " + sProgressPercentage;
                LCDController.FirmwareUpdateProgress(sProgressPercentage);
                WifiManager.uiLastProgress = iProgressPercentage;
            } });
        ArduinoOTA.onError([](ota_error_t error){
            Serial.printf("Error[%u]: ", error);
            LCDController.FirmwareUpdateError();
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
        ArduinoOTA.begin();
        
        mdnsServerSetup();

    }
    else if (iOpMode == SystemModes::BLUE)
    {
        _strSTAName = _strAPName;
        _strAPName += "_BLUE";
        _IPGateway = IPAddress(192, 168, 21, 1);
        _IPSubnet = IPAddress(255, 255, 255, 0);
        if (!WiFi.mode(WIFI_MODE_APSTA) || !WiFi.softAP(_strAPName.c_str(), _strAPPass.c_str()))
        {
            log_w("Error initializing softAP with name %s!", _strAPName.c_str());
        }
        WiFi.begin(_strSTAName.c_str(), _strAPPass.c_str());
    }
    else
        log_e("Got unknown mode, no idea how I should start...");
}

void WifiManagerClass::WiFiLoop()
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

void WifiManagerClass::WiFiEvent(arduino_event_id_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_START:
        log_i("Configuring AP...");
        if (!WiFi.softAPConfig(_IPGateway, _IPGateway, _IPSubnet))
        {
            log_e("AP configuration failed!");
        }
        else
        {
            log_i("AP configuration OK, ssid: %s, IP: %s", _strAPName.c_str(), WiFi.softAPIP().toString().c_str());
        }
        if (WiFi.softAPIP() != _IPGateway)
        {
            log_e("I am not running on the correct IP (%s instead of %s), rebooting!", WiFi.softAPIP().toString().c_str(), _IPGateway.toString().c_str());
            ESP.restart();
        }
        break;

    case ARDUINO_EVENT_WIFI_AP_STOP:
        log_i("AP stopped");
        break;

    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        // log_i("IP assigned to new client");
        break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        // bCheckWsClinetStatus = true;
        // ipTocheck = IPAddress (192,168,20,2);
        // log_i("IP to check: %s", ipTocheck.toString().c_str());
        break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        //log_i("STA got IP %s", WiFi.localIP().toString().c_str());
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
        log_i("Connected to RED ETS AP %s", WiFi.SSID().c_str());
        break;

    default:
        //log_d("Wifi event %i", event);
        break;
    }
}
void WifiManagerClass::mdnsServerSetup()
{
    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("arduino", "tcp", "app_version", APP_VER);
    MDNS.begin("flyballets");
}

void WifiManagerClass::ToggleWifi()
{
   if (WiFi.getMode() == WIFI_MODE_AP)
   {
      WiFi.mode(WIFI_MODE_NULL);
      LCDController.UpdateField(LCDController.WifiState, " ");
      LCDController.bExecuteLCDUpdate = true;
      log_i("WiFi OFF");
   }
   else
   {
      WiFi.mode(WIFI_MODE_AP);
      LCDController.UpdateField(LCDController.WifiState, "W");
      LCDController.bExecuteLCDUpdate = true;
      log_i("WiFi ON");
   }
}

WifiManagerClass WifiManager;