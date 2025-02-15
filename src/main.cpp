// file:	main.cpp summary: FlyballETS-Software by simonttp78 forked from Alex Goris
//
// Flyball ETS (Electronic Training System) is an open source project which is designed to help
// teams who practice flyball (a dog sport). Read about original project, including extensive
// information on first prototype version of Flyball ETS, on the following link: https://
// sparkydevices.wordpress.com/tag/flyball-ets/
//
// This part of the project (FlyballETS-Software) contains the ESP32 source code for the ESP32
// LoLin32 which controls all components in the Flyball ETS These sources are originally
// distributed from: https://github.com/vyruz1986/FlyballETS-Software.
//
// Copyright (C) 2019 Alex Goris
// This file is part of FlyballETS-Software
// FlyballETS-Software is free software : you can redistribute it and / or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program.If not,
// see <http://www.gnu.org/licenses/>
#include "main.h"

// Declare WS2811B compatibile lights strip
NeoPixelBus<NeoRgbFeature, WS_METHOD> LightsStrip(5 * LIGHTSCHAINS, iLightsDataPin);

// Declare 40x4 LCD by 2 virtual LCDes
LiquidCrystal lcd1(iLCDRSPin, iLCDE1Pin, iLCDData4Pin, iLCDData5Pin, iLCDData6Pin, iLCDData7Pin);  // this will be line 1&2 of 40x4 LCD
LiquidCrystal lcd2(iLCDRSPin, iLCDE2Pin, iLCDData4Pin, iLCDData5Pin, iLCDData6Pin, iLCDData7Pin); // this will be line 3&4 of 40x4 LCD

// IP addresses declaration
IPAddress IPGateway(192, 168, 20, 1);
IPAddress IPNetwork(192, 168, 20, 0);
IPAddress IPSubnet(255, 255, 255, 0);

// Statically allocate and initialize the spinlock
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;

void setup()
{
   Serial.begin(115200);
   SettingsManager.init();

   // Configure sensors pins
   pinMode(iS1Pin, INPUT_PULLDOWN); // ESP32 has no pull-down resistor on pin 34, but it's pulled-down anyway by 1kohm resistor in voltage leveler circuit
   pinMode(iS2Pin, INPUT_PULLDOWN);

   // Initialize lights
   pinMode(iLightsDataPin, OUTPUT);

   // Configure pins for shift register
   pinMode(iLatchPin, OUTPUT);
   pinMode(iClockPin, OUTPUT);
   pinMode(iDataInPin, INPUT_PULLDOWN);

   // Configure pins for SD Card
   pinMode(iSDdata0Pin, INPUT_PULLUP);
   pinMode(iSDdata1Pin, INPUT_PULLUP);
   pinMode(iSDcmdPin, INPUT_PULLUP);
   pinMode(iSDdetectPin, INPUT_PULLUP);

   // Configure LCD pins
   pinMode(iLCDData4Pin, OUTPUT);
   pinMode(iLCDData5Pin, OUTPUT);
   pinMode(iLCDData6Pin, OUTPUT);
   pinMode(iLCDData7Pin, OUTPUT);
   pinMode(iLCDE1Pin, OUTPUT);
   pinMode(iLCDE2Pin, OUTPUT);
   pinMode(iLCDRSPin, OUTPUT);

   // Set ISR's with wrapper functions
#if !Simulate
   attachInterrupt(digitalPinToInterrupt(iS1Pin), Sensor1Wrapper, CHANGE);
   attachInterrupt(digitalPinToInterrupt(iS2Pin), Sensor2Wrapper, CHANGE);
#endif

   // Configure Laser output pin
   pinMode(iLaserOutputPin, OUTPUT);

   // Configure GPS PPS pin
   pinMode(iGPSppsPin, INPUT_PULLDOWN);

   // Print SW version
   Serial.printf("Firmware version: %s\r\n", FW_VER);
   Serial.printf("FW compilation date: %s\r\n",__DATE__);

   // Initialize BatterySensor class with correct pin
   BatterySensor.init(iBatterySensorPin);

   // Initialize LightsController class
   xTaskCreatePinnedToCore(
      Core1Lights,
      "Lights",
      8192,
      NULL,
      1,
      &taskLights,
      1);

   // Initialize LCDController class with lcd1 and lcd2 objects
   LCDController.init(&lcd1, &lcd2);

   strSerialData[0] = 0;

   // Initialize GPS
   GPSHandler.init(iGPSrxPin, iGPStxPin);
   
   // SD card init
   if (digitalRead(iSDdetectPin) == LOW)
      SDcardController.init();
   else
      Serial.println("SD Card not inserted!");

   // Initialize RaceHandler class with S1 and S2 pins
   xTaskCreatePinnedToCore(
      Core1Race,
      "Race",
      16384,
      NULL,
      1,
      &taskRace,
      1);

#ifdef WiFiON
   // Setup AP
   WiFi.onEvent(WiFiEvent);
   WiFi.mode(WIFI_MODE_AP);
   String strAPName = SettingsManager.getSetting("APName");
   String strAPPass = SettingsManager.getSetting("APPass");
   if (!WiFi.softAP(strAPName.c_str(), strAPPass.c_str()))
      log_e("Error initializing softAP!");
   else
      log_i("Wifi started successfully, AP name: %s, pass: %s", strAPName.c_str(), strAPPass.c_str());

   // configure webserver
   WebHandler.init(80);
   mdnsServerSetup();
#endif

   iLaserOnTime = atoi(SettingsManager.getSetting("LaserOnTimer").c_str());
   log_i("Configured laser ON time: %is", iLaserOnTime);

   log_w("ESP log level %i", CORE_DEBUG_LEVEL);
}

void loop()
{
   if (!WebHandler.bFwUpdateInProgress)
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
      {
         SettingsManager.loop();
         GPSHandler.loop();
         BatterySensor.CheckBatteryVoltage();
         SDcardController.CheckSDcardSlot(iSDdetectPin);
      }

      serialEvent();

      if (bSerialStringComplete)
         HandleSerialCommands();

      HandleRemoteAndButtons();
   }
   else
   {
      vTaskSuspend(taskRace);
      vTaskSuspend(taskLights);
   }

   LCDController.Main();

#ifdef WiFiON
   WebHandler.loop();
#endif
}

void serialEvent()
{
   // Listen on serial port
   while (Serial.available() > 0)
   {
      char cInChar = Serial.read();
      if (cInChar == '\n')
      {
         bSerialStringComplete = true;
         log_d("SERIAL received: '%s'", strSerialData.c_str());
         strSerialData += '\0';
         break;
      }
      strSerialData += cInChar;
   }
}

/// <summary>
///   These are wrapper functions which are necessary because it's not allowed to use a class member function directly as an ISR
/// </summary>
void IRAM_ATTR Sensor1Wrapper()
{
   RaceHandler.TriggerSensor1(&spinlock);
}

/// <summary>
///   These are wrapper functions which are necessary because it's not allowed to use a class member function directly as an ISR
/// </summary>
void IRAM_ATTR Sensor2Wrapper()
{
   RaceHandler.TriggerSensor2(&spinlock);
}

/// <summary>
///   Start a race.
/// </summary>
void StartRaceMain()
{
   if (RaceHandler.RaceState != RaceHandler.RESET)
      return;
   if (LightsController.bModeNAFA)
      LightsController.WarningStartSequence();
   else
      LightsController.InitiateStartSequence();
}

/// <summary>
///   Stop a race.
/// </summary>
void StopRaceMain()
{
   if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
      return;
   RaceHandler.bExecuteStopRace = true;
   LightsController.DeleteSchedules();
}

/// <summary>
///   Starts (if stopped) or stops (if started) a race. Start is only allowed if race is stopped and reset.
/// </summary>
void StartStopRace()
{
   if (RaceHandler.RaceState == RaceHandler.RESET)
      StartRaceMain();
   else
      StopRaceMain();
}

/// <summary>
///   Reset race so new one can be started, reset is only allowed when race is stopped
/// </summary>
void ResetRace()
{
   if (RaceHandler.RaceState != RaceHandler.STOPPED)
      return;
   RaceHandler.bExecuteResetRace = true;
   LightsController.bExecuteResetLights = true;
}

#ifdef WiFiON
void WiFiEvent(arduino_event_id_t event)
{
   switch (event)
   {
   case ARDUINO_EVENT_WIFI_AP_START:
      WiFi.softAPConfig(IPGateway, IPGateway, IPSubnet);
      if (WiFi.softAPIP() != IPGateway)
      {
         log_e("I am not running on the correct IP (%s instead of %s), rebooting!", WiFi.softAPIP().toString().c_str(), IPGateway.toString().c_str());
         ESP.restart();
      }
      log_i("Ready on IP: %s, v%s", WiFi.softAPIP().toString().c_str(), APP_VER);
      break;

   case ARDUINO_EVENT_WIFI_AP_STOP:
      break;

   case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      break;

   case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      break;

   default:
      break;
   }
}

void ToggleWifi()
{
   if (WiFi.getMode() == WIFI_MODE_AP)
   {
      WiFi.mode(WIFI_OFF);
      LCDController.UpdateField(LCDController.WifiState, " ");
      LCDController.bExecuteLCDUpdate = true;
      log_i("WiFi OFF");
   }
   else
   {
      WiFi.mode(WIFI_AP);
      LCDController.UpdateField(LCDController.WifiState, "W");
      LCDController.bExecuteLCDUpdate = true;
      log_i("WiFi ON");
   }
}

void mdnsServerSetup()
{
   if (!MDNS.begin("flyballets")) {
      Serial.println("Error setting up MDNS responder!");
      return;
   }
   log_i("mDNS responder started");
   MDNS.addService("http", "tcp", 80);
}
#endif

void HandleSerialCommands()
{
   // Race start
   if (strSerialData == "start")
      StartRaceMain();
   // Race stop
   if (strSerialData == "stop")
      StopRaceMain();
   // Race reset button
   if (strSerialData == "reset")
      ResetRace();
   // Print time
   if (strSerialData == "time")
      log_i("System time:  %s", GPSHandler.GetLocalTimestamp());
   // Print uptime
   if (strSerialData == "uptime")
   {
      uint32_t t = (uint32_t)(millis() / 1000);
      uint8_t s = t % 60;
      t = (t - s) / 60;
      uint8_t m = t % 60;
      t = (t - m) / 60;
      uint16_t h = t;
      log_i("Up time: %i:%i:%i", h, m, s);
   }
   // Delete tag file
   if (strSerialData == "deltagfile")
      SDcardController.deleteFile(SD_MMC, "/tag.txt");
   // List files on SD card
   if (strSerialData == "list")
   {
      SDcardController.listDir(SD_MMC, "/", 0);
      SDcardController.listDir(SD_MMC, "/SENSORS_DATA", 0);
   }
   // Reboot ESP32
   if (strSerialData == "reboot")
      ESP.restart();
   // Prepare for automatic tests
   if (strSerialData == "preparefortesting")
      if (!Simulate)
         log_e("FAILED - Firmware's not compiled in Simulation mode");
      else
      {
         if (SettingsManager.getSetting("Accuracy3digits").equals("0"))
            RaceHandler.ToggleAccuracy();
         if (SettingsManager.getSetting("RunDirectionInverted").equals("1"))
            RaceHandler.ToggleRunDirection();
         log_i("DONE - Simulation mode active. Accuracy set to 3. Run direction: normal.");
      }
#if Simulate
   if (strSerialData.startsWith("race"))
   {
      strSerialData.remove(0, 5);
      Simulator.iSimulatedRaceID = strSerialData.toInt();
      if (Simulator.iSimulatedRaceID < 0 || Simulator.iSimulatedRaceID >= NumSimulatedRaces)
      {
         Simulator.iSimulatedRaceID = 0;
      }
      Simulator.bExecuteSimRaceChange = true;
   }
#endif
   // Dog 1 fault
   if (strSerialData == "d1f")
      RaceHandler.SetDogFault(0);
   // Dog 2 fault
   if (strSerialData == "d2f")
      RaceHandler.SetDogFault(1);
   // Dog 3 fault
   if (strSerialData == "d3f")
      RaceHandler.SetDogFault(2);
   // Dog 4 fault
   if (strSerialData == "d4f")
      RaceHandler.SetDogFault(3);
   // Toggle race direction
   if (strSerialData == "direction")
      RaceHandler.ToggleRunDirection();
   // Set explicitly number of racing dogs
   if (strSerialData.startsWith("setdogs") && RaceHandler.RaceState == RaceHandler.RESET)
   {
      strSerialData.remove(0, 8);
      uint8_t iNumberofRacingDogs = strSerialData.toInt();
      if (iNumberofRacingDogs < 1 || iNumberofRacingDogs > 4)
      {
         iNumberofRacingDogs = 4;
      }
      RaceHandler.SetNumberOfDogs(iNumberofRacingDogs);
   }
   // Toggle accuracy
   if (strSerialData == "accuracy")
      RaceHandler.ToggleAccuracy();
   // Toggle decimal separator in CSV
   if (strSerialData == "separator")
      SDcardController.ToggleDecimalSeparator();
   // Toggle between modes
   if (strSerialData == "mode")
      LightsController.ToggleStartingSequence();
   // Reruns off
   if (strSerialData == "reruns off")
      RaceHandler.ToggleRerunsOffOn(1);
   // Reruns on
   if (strSerialData == "reruns on")
      RaceHandler.ToggleRerunsOffOn(0);
   // Toggle wifi on/off
   if (strSerialData == "wifi")
      ToggleWifi();
   // Toggle wifi on/off
   if (strSerialData == "fwver")
      Serial.printf("Firmware version: %s\r\n", FW_VER);
   // Factory Reset
   if (strSerialData == "factoryreset")
      FactoryReset();

   // Make sure this stays last in the function!
   if (strSerialData.length() > 0)
   {
      strSerialData = "";
      bSerialStringComplete = false;
   }
}

void HandleRemoteAndButtons()
{
   byDataIn = 0;
   digitalWrite(iLatchPin, LOW);
   digitalWrite(iClockPin, LOW);
   digitalWrite(iClockPin, HIGH);
   digitalWrite(iLatchPin, HIGH);
   for (uint8_t i = 0; i < 8; ++i)
   {
      byDataIn |= digitalRead(iDataInPin) << (7 - i);
      digitalWrite(iClockPin, LOW);
      digitalWrite(iClockPin, HIGH);
   }
   if (byDataIn != 0 && byDataIn != 1 && byDataIn != 2 && byDataIn != 4 && byDataIn != 8 && byDataIn != 16 && byDataIn != 32 && byDataIn != 64 && byDataIn != 128)
   {
      byDataIn = 0;
   }
   if (byDataIn != byLastFlickerableState)
   {
      llLastDebounceTime = millis();
      byLastFlickerableState = byDataIn;
   }
   if ((byLastStadyState != byDataIn) && ((millis() - llLastDebounceTime) > DEBOUNCE_DELAY))
   {
      if (byDataIn != 0)
         iLastActiveBit = log2(byDataIn & -byDataIn);
      if (bitRead(byLastStadyState, iLastActiveBit) == LOW && bitRead(byDataIn, iLastActiveBit) == HIGH)
         llPressedTime[iLastActiveBit] = millis();
      else if (bitRead(byLastStadyState, iLastActiveBit) == HIGH && bitRead(byDataIn, iLastActiveBit) == LOW)
         llReleasedTime[iLastActiveBit] = millis();
      byLastStadyState = byDataIn;
      long long llPressDuration = (llReleasedTime[iLastActiveBit] - llPressedTime[iLastActiveBit]);
      if (llPressDuration > 0)
      {
         if (iLastActiveBit == 1)
            StartStopRace();
         else if (iLastActiveBit == 2)
            ResetRace();
         if (llPressDuration <= SHORT_PRESS_TIME)
         {
            log_d("%s SHORT press detected: %lldms", GetButtonString(iLastActiveBit).c_str(), llPressDuration);
            if (iLastActiveBit == 3)
               if (RaceHandler.RaceState == RaceHandler.RESET)
                  RaceHandler.SetNumberOfDogs(1);
               else
                  RaceHandler.SetDogFault(0);
            else if (iLastActiveBit == 6)
               if (RaceHandler.RaceState == RaceHandler.RESET)
                  RaceHandler.SetNumberOfDogs(2);
               else
                  RaceHandler.SetDogFault(1);
            else if (iLastActiveBit == 5)
               if (RaceHandler.RaceState == RaceHandler.RESET)
                  RaceHandler.SetNumberOfDogs(3);
               else
                  RaceHandler.SetDogFault(2);
            else if (iLastActiveBit == 4)
               if (RaceHandler.RaceState == RaceHandler.RESET)
                  RaceHandler.SetNumberOfDogs(4);
               else
                  RaceHandler.SetDogFault(3);
            else if (iLastActiveBit == 0 && (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET))
               RaceHandler.ToggleAccuracy();
            else if (iLastActiveBit == 7 && !bLaserActive && (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET))
            {
               digitalWrite(iLaserOutputPin, HIGH);
               bLaserActive = true;
               log_i("Turn Laser ON.");
            }
         }
         else if (llPressDuration > SHORT_PRESS_TIME && llPressDuration <= VERYLONG_PRESS_TIME)
         {
            log_d("%s LONG press detected: %lldms", GetButtonString(iLastActiveBit).c_str(), llPressDuration);
            if (iLastActiveBit == 3) 
               RaceHandler.ToggleRerunsOffOn(2);
            else if (iLastActiveBit == 6 && RaceHandler.RaceState == RaceHandler.RESET)
               LightsController.ToggleStartingSequence();
            else if (iLastActiveBit == 0)
               RaceHandler.ToggleRunDirection();
            else if (iLastActiveBit == 7 && RaceHandler.RaceState == RaceHandler.RESET)
               ToggleWifi();
         }
         else if (llPressDuration > VERYLONG_PRESS_TIME)
         {
            log_d("%s VERY LONG press detected: %lldms", GetButtonString(iLastActiveBit).c_str(), llPressDuration);
            if (iLastActiveBit == 7 && RaceHandler.RaceState == RaceHandler.RESET)
               FactoryReset();
         }
      }
   }
   if ((bLaserActive) && ((millis() - llReleasedTime[7] > iLaserOnTime * 1000) || RaceHandler.RaceState == RaceHandler.STARTING || RaceHandler.RaceState == RaceHandler.RUNNING))
   {
      digitalWrite(iLaserOutputPin, LOW);
      bLaserActive = false;
      log_i("Turn Laser OFF.");
   }
}

/// <summary>
///   Factory Reset - erasing and initializing NVM.
/// </summary>
void FactoryReset()
{
   Serial.println("Trying to erse all NVS flash...");
   if (nvs_flash_erase() != ESP_OK) Serial.println("===> Error with Flash Erase.");
   if (nvs_flash_init() != ESP_OK) Serial.println("===> Error with Flash INIT.");
   vTaskDelay(1000);
   ESP.restart();
}


/// <summary>
///   Gets pressed button string for consol printing.
/// </summary>
String GetButtonString(uint8_t _iActiveBit)
{
   String strButton;
   switch (_iActiveBit)
   {
   case 0:
      strButton = "Mode button";
      break;
   case 1:
      strButton = "Remote 1: start/stop";
      break;
   case 2:
      strButton = "Remote 2: reset";
      break;
   case 3:
      strButton = "Remote 3: dog 1 fault";
      break;
   case 6:
      strButton = "Remote 6: dog 4 fault";
      break;
   case 5:
      strButton = "Remote 5: dog 3 fault";
      break;
   case 4:
      strButton = "Remote 4: dog 2 fault";
      break;
   case 7:
      strButton = "Laser trigger";
      break;
   default:
      strButton = "Unknown --> Ingored";
      break;
   }

   return strButton;
}

void Core1Race(void *parameter)
{
#if Simulate
   Simulator.init();
#endif
   RaceHandler.init(iS1Pin, iS2Pin);
   for (;;)
   {
   #if Simulate
      Simulator.Main();
   #endif
      RaceHandler.Main();
      vTaskDelay(1 / portTICK_PERIOD_MS);
   }
}

void Core1Lights(void *parameter)
{
   LightsController.init(&LightsStrip);
   for (;;)
   {
      LightsController.Main();
      vTaskDelay(1 / portTICK_PERIOD_MS);
   }
}


