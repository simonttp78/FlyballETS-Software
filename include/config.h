// Copyright (C) 2019 Alex Goris
// This file is part of FlyballETS-Software
// FlyballETS-Software is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>

#ifndef GLOBALCONST_H
#define GLOBALCONST_H

#define MICROS esp_timer_get_time()

#undef CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH
#undef CONFIG_ESP_COREDUMP_CHECK_BOOT
#define CONFIG_ESP_COREDUMP_ENABLE_TO_UART 1

#define FW_VER "1.17.24 "         // Flyball ETS firmware version

#define Simulate false            // Set to true to enable race simulation (see Simulator.h/.cpp)
#define NumSimulatedRaces 67      // Number of prepeared simulated races. Sererial interface command to change interface: e.g. "race 1"
#define TRIGGER_QUEUE_LENGTH 110  // Number of triggers in the queue

#define WiFiON                    // If defined all WiFi features are on: OTA, Web server. Please be carefull. Keep remote receiver board (antenna) away from ESP32 to avoid interferences.
//#define WebUIonSDcard           // When defined webserver data will be loaded from SC card (MMC 1 bit mode) and not taken compiled into fimrware package. Precondition: SDcard defined too.
#define BatteryCalibration false  // after setting to true LCD will display analog read value from battery pin (range 0-4095). This is handfull for battery volate curve definition (dPinVoltage)

#define LIGHTSCHAINS 1            // Numer of WS281x lights chains. 1 - one chain of 5 pixels/lights, 2 - two chains --> 10 pixels/lights, etc.
#define WS_METHOD NeoWs2812xMethod

#define EEPROM_SIZE 4096          // EEPROM size in bytes
#define SPI_FLASH_SEC_SIZE 4096   // Flash Sector Size declaration for ESP32 as it seems to become removed from embedded libraries
#define U_PART U_SPIFFS

#define WS_TICKET_BUFFER_SIZE 8   // Number of websocket tickets kept in memory
#define WS_TIMEOUT 1800000        // Timeout for secured websocket in miliseconds

#define APP_VER "1.1.1"           // WebUI version

#endif