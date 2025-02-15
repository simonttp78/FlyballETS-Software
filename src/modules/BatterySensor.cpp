// file:	BatterySensor.cpp
//
// summary:	Implements the battery sensor class
// Copyright (C) 2019 Alex Goris
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
#include "BatterySensor.h"
#include "config.h"

/// <summary>
///   Initialises this object.
/// </summary>
///
/// <param name="iBatterySensorPin">   Zero-based index of the battery sensor pin. </param>
void BatterySensorClass::init(uint8_t iBatterySensorPin)
{
   pinMode(iBatterySensorPin, INPUT);
   _iBatterySensorPin = iBatterySensorPin;
   _iNumberOfBatteryReadings = 0;
}

/// <summary>
///   Will check the current battery voltage. 10 readings will be taken to smooth out fluctuations.
/// </summary>
void BatterySensorClass::CheckBatteryVoltage()
{
   if (_iNumberOfBatteryReadings < 10)
   {
      _iBatteryReadings[_iNumberOfBatteryReadings] = analogRead(_iBatterySensorPin);
      _iNumberOfBatteryReadings++;
   }
   else
   {
      int iBatteryReadingsTotal = 0;
      for (int i = 0; i < _iNumberOfBatteryReadings; i++)
      {
         iBatteryReadingsTotal = iBatteryReadingsTotal + _iBatteryReadings[i];
      }
      _iAverageBatteryReading = iBatteryReadingsTotal / _iNumberOfBatteryReadings;

      //First calculate voltage at ADC pin
      //int iPinVoltage = map(_iAverageBatteryReading, 958, 4095, 916, 3150);
      double dPinVoltage = (-0.00012493) * pow(_iAverageBatteryReading, 2) + 1.4559 * _iAverageBatteryReading - 671.7;
      int iPinVoltage = dPinVoltage;
      _iBatteryVoltage = iPinVoltage * 4.3172;
      _iNumberOfBatteryReadings = 0;
      //log_d("_iBatteryVoltage: %i", _iBatteryVoltage);
   }
}

/// <summary>
///   Gets battery voltage.
/// </summary>
///
/// <returns>
///   The battery voltage.
/// </returns>
uint16_t BatterySensorClass::GetBatteryVoltage()
{
   return _iBatteryVoltage;
}

/// <summary>
///   Gets battery percentage or analog pin read if calibrqtion mode.
///   Assumed working range is 10.5V - 12.3V what is save for 3S2P and 3S4P li-ion batteries

/// </summary>
uint16_t BatterySensorClass::GetBatteryPercentage()
{
#if BatteryCalibration
   return _iAverageBatteryReading;
#else
   if (_iBatteryVoltage < 5000 || _iBatteryVoltage >= 60000)
   {
      return 9911;
   }
   else if (_iBatteryVoltage >= 5000 && _iBatteryVoltage < 10000)
   {
      return 9999;
   }
   else if (_iBatteryVoltage >= 10000 && _iBatteryVoltage < 10500)
   {
      return 0;
   }
   else if (_iBatteryVoltage > 12250 && _iBatteryVoltage < 60000)
   {
      return 100;
   }
   else
   {
      uint16_t iBatteryPercentage = map(_iBatteryVoltage, 10500, 12250, 1, 100);
      return iBatteryPercentage;
   }
#endif
}

/// <summary>
///   Gets the last analogRead value from battery sense pin.
/// </summary>
///
/// <returns>
///   A value from 0-4095
/// </returns>
uint16_t BatterySensorClass::GetLastAnalogRead()
{
   return _iAverageBatteryReading;
}

/// <summary>
///   The battery sensor.
/// </summary>
BatterySensorClass BatterySensor;