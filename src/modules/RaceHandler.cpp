//  file:	RaceHandler.cpp
//
// summary:	Implements the race handler class
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

#include "LightsController.h"
#include "LCDController.h"
#include "RaceHandler.h"
#include "SettingsManager.h"
#include "config.h"
#include "WebHandler.h"
#include "SDcardController.h"

/// <summary>
///   Initialises this object andsets all counters to 0.
/// </summary>
///
/// <param name="iS1Pin">  Zero-based index of the S1 pin. </param>
/// <param name="iS2Pin">  Zero-based index of the S2 pin. </param>
void RaceHandlerClass::init(uint8_t iS1Pin, uint8_t iS2Pin)
{
   // Start in ready/reset state
   _iS1Pin = iS1Pin;
   _iS2Pin = iS2Pin;
   if (SettingsManager.getSetting("RunDirectionInverted").equals("1"))
   {
      bRunDirectionInverted = true;
      LCDController.UpdateField(LCDController.BoxDirection, "<");
      log_i("Run direction from settings: inverted");
   }
   else
      log_i("Run direction from settings: normal");

   if (SettingsManager.getSetting("Accuracy3digits").equals("1"))
   {
      _bAccuracy3digits = true;
      log_i("Accuracy from settings: 3 digits");
   }
   else
      log_i("Accuracy from settings: 2 digits");
   LCDController.bUpdateTimerLCDdata = true;
   LCDController.bExecuteLCDUpdate = true;
#ifdef WiFiON
   WebHandler.bUpdateRaceData = true;
   WebHandler.bSendRaceData = true;
#endif
}

/// <summary>
///   Main entry-point for this application. This function should be called once every main loop.
///   It will check if any new interrupts were saved from the sensors, and handle them if this
///   the case. All timing related data and also fault handling of the dogs is done in this
///   function.
/// </summary>
void RaceHandlerClass::Main()
{
   NOW = MICROS;
   // Check if race state should be changed to RUNNING
   if (RaceState == STARTING && NOW >= llRaceStartTime)
   {
      _ChangeRaceState(RUNNING);
      // log_d("GREEN light is ON!");
   }

   if (_bRaceStopRequested)
   {
      if (NOW - _llRaceEndTime >= 2000000)
      {
         _bRaceStopRequested = false;
         _ChangeRaceState(STOPPED);
      }
      else if (_bDogManualFaults[iCurrentDog])
      {
         _bRaceStopRequested = false;
         iDogRunCounters[iCurrentDog]++;
      }
   }

   if (RaceState == RUNNING && !_bRaceStopRequested)
   {
      if ((NOW > llRaceStartTime) && (NOW - llRaceStartTime > _llRaceTime + 227300))
      {
         _llRaceTime = NOW - llRaceStartTime;
         LCDController.bUpdateThisLCDField[LCDController.TeamTime] = true;
         LCDController.bUpdateThisLCDField[iCurrentDog] = true;
#ifdef WiFiON
         WebHandler.bUpdateThisRaceDataField[WebHandler.elapsedTime] = true;
         WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
      }

      if (_llRaceTime > 600000000)
      {
         log_w("Race TIMEOUT!!!");
         StopRace(_llRaceTime);
      }
   }

   NOW = MICROS;
   if ((_iInputQueueReadIndex != _iInputQueueWriteIndex) && ((NOW - _InputTriggerQueue[_iInputQueueWriteIndex - 1].llTriggerTime) > 12000))
   {
      log_v("IQRI:%d | IQWI:%d | Delta:%lld | RaceTime:%lld", _iInputQueueReadIndex, _iInputQueueWriteIndex, NOW - _InputTriggerQueue[_iInputQueueWriteIndex - 1].llTriggerTime, NOW - llRaceStartTime);
      _QueueFilter();
   }

   if (bExecuteStopRace)
   {
      bRaceStoppedManually = true;
      this->StopRace(MICROS);
      bExecuteStopRace = false;
   }

   if (bExecuteResetRace)
   {
      ResetRace();
      bExecuteResetRace = false;
   }

   if (bExecuteStartRaceTimer)
   {
      StartRaceTimer();
      bExecuteStartRaceTimer = false;
   }

   if (_bClearCurrentDogFault && (NOW - _llClearFaultTime) > 300000)
   {
      SetDogFault(iCurrentDog, OFF);
      _bClearCurrentDogFault = false;
   }

   if (!_QueueEmpty()) // If queue is not empty, we have work to do
   {
      // Get next record from queue
      STriggerRecord STriggerRecord = _QueuePop();

      // If the transition string is not empty check if we shall apply aditional filtering
      if (_strTransition.length() != 0)
      {
         if (_bPotentialNegativeCrossDetected && (NOW - _llS2CrossedUnsafeGetMicrosTime) > 100000)
         {
            _bPotentialNegativeCrossDetected = false;
            log_d("Potential negative cross flag reset as S1 not crossed for 100ms");
            _ClearTransitionString();
         }

         if (_byDogState == GOINGIN)
         {
            if ((NOW - _llLastTransitionStringUpdate) > 270000)
            {
               if (_strTransition.substring(_strTransition.length() - 1) == "b" && _strPreviousTransitionFirstLetter == "B")
               {
                  _ChangeDogState(COMINGBACK);
                  _bS1StillSafe = false;
                  _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
                  _bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                  LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
               #ifdef WiFiON
                  WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
               #endif
                  log_d("Seems dog %i entered gate already as S2 state 'b' detected. 'ok' crossing. S1 is not safe anymore.", iCurrentDog + 1);
               }
               else
               {
                  _bS1StillSafe = true;
                  log_d("False entering dog detected. S1 is safe.");
               }
            _ClearTransitionString();
            }

            else if (!_bGatesClear && (NOW - _llGatesClearedTime) < 50000 && _strTransition.length() == 1 && _strTransition.substring(0) == "B")
            {
               _bS1StillSafe = true;
               log_d("S2 sensor noise detected after comming back dog. Gates clear.");
               _ClearTransitionString();
            }
         }
         else // COMINGBACK
         {
            if (_strTransition.length() == 2 && _strTransition.substring(0) == "Aa" && !(iCurrentDog == 0 && !_bRerunBusy) && ((NOW - _llLastTransitionStringUpdate) > 100000) && ((_llDogEnterTimes[iCurrentDog] - _llDogEnterTimes[iPreviousDog]) > 2000000))
            {
               if (!_bDogFaults[iCurrentDog] && !_bPrepareToRestoreokCrossing)
                  _ChangeDogState(GOINGIN);
               if (_bClearCurrentDogFault)
                  _bClearCurrentDogFault = false;
               if (_bPrepareToRestoreokCrossing)
               {
                  if (_bWasItBigOK)
                  {
                     _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                     _bWasItBigOK = false;
                  }
                  else
                     _bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;

                  _llDogEnterTimes[iCurrentDog] = _llDogExitTimes[iPreviousDog];
                  _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
                  _bPrepareToRestoreokCrossing = false;
                  LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
               #ifdef WiFiON
                  WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
               #endif
                  _bPrepareToRestoreokCrossing = false;
                  _bS1StillSafe = false;
                  log_d("It wasn't 'false ok/OK' crossing. Restoring 'ok/OK' for dog %i.", iCurrentDog + 1);
               }
               else
               {
                  _bS1StillSafe = true;
                  log_d("False entering dog detected with 'Aa' noise. S1 is safe.");
               }
               _ClearTransitionString();
            }
            else if ((NOW - _llLastTransitionStringUpdate) > 270000)
            {
               _bS1StillSafe = false;
               log_d("Noise detected on S1. S1 is not safe anymore.");
               _ClearTransitionString();
            }
         }
      }

      log_d("S%i | TT:%lld | T:%lld | St:%i", STriggerRecord.iSensorNumber, STriggerRecord.llTriggerTime, STriggerRecord.llTriggerTime - llRaceStartTime, STriggerRecord.iSensorState);

      // Calculate what our next dog will be
      uint8_t iNextDogChanged = iNextDog;
      if (_bRerunNeeded && iCurrentDog == (iNumberOfRacingDogs - 1) && iNextDog < 5)
      {
         if (bRerunsOff)
            iNextDog = 5;
         else
         {
            for (uint8_t i = 0; i < iNumberOfRacingDogs; i++)
            {
               if (_bDogFaults[i] || _bDogManualFaults[i])
               {
                  iNextDog = i;
                  break;
               }
            }
         }
      }
      else if (_bRerunNeeded && _bRerunBusy && !bRerunsOff)
      {
         for (uint8_t i = (iCurrentDog + 1); i < iNumberOfRacingDogs; i++)
         {
            if (_bDogFaults[i] || _bDogManualFaults[i])
            {
               iNextDog = i;
               _bNextDogFound = true;
               break;
            }
         }
         if (!_bNextDogFound)
         {
            for (uint8_t i = 0; (iCurrentDog + 1); i++)
            {
               if (_bDogFaults[i] || _bDogManualFaults[i])
               {
                  iNextDog = i;
                  _bNextDogFound = false;
                  break;
               }
            }
         }
         _bNextDogFound = false;
      }
      else if ((!_bRerunNeeded && iCurrentDog == (iNumberOfRacingDogs - 1)) || (!_bRerunNeeded && _bRerunBusy))
         iNextDog = iCurrentDog;
      else if (iNextDog != iCurrentDog + 1 && iNextDog < 5)
         iNextDog = iCurrentDog + 1;
      if (iNextDogChanged != iNextDog)
         log_d("Next Dog is %i.", iNextDog + 1);


      //--------------------------------------------------------------------------------------------------------------------
      // Handle SENSOR 1 events (handlers side) with gates CLEAR
      if (STriggerRecord.iSensorNumber == 1 && STriggerRecord.iSensorState == 1 && _bGatesClear && iCurrentDog < 5) // Only if gates are clear and S1 sensor is HIGH (A)
      {
         _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
         // Special handling for first dog during race start (excluding possible re-run)
         if (_byDogState == GOINGIN && iCurrentDog == 0 && !_bRerunBusy)
         {
            _llDogEnterTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - llRaceStartTime;
            if (STriggerRecord.llTriggerTime < llRaceStartTime)
            {
               SetDogFault(iCurrentDog, ON);
               log_i("Dog 1 False start!");
            }
            LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
            WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
         }
         // Normal race handling (positive cross)
         else if (_byDogState == GOINGIN && (iCurrentDog != 0 || (iCurrentDog == 0 && _bRerunBusy)) && _bS1StillSafe && !_bRaceStopRequested)
         {
             if (!_bRerunBusy && _bLastStringBAba && (STriggerRecord.llTriggerTime - _llLastDogExitTime) > 3500000 //
                  && (STriggerRecord.llTriggerTime - _llLastDogExitTime) < 5500000 && iCurrentDog != iNextDog && iCurrentDog < 3 && iNextDog < 5)
            {
               SetDogFault(iNextDog, ON);
               _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
               _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
               _llLastDogExitTime = _llDogExitTimes[iCurrentDog];
               _llDogTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogExitTimes[iCurrentDog] - _llDogEnterTimes[iCurrentDog];
               _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
               if (_bDogManualFaults[iCurrentDog])
               {
                  _bDogMissedGateGoingin[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                  _bNoValidCrossingTime[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               }
               else
                  _bDogInvisibleOk[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
               WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
               _ChangeDogNumber(iNextDog);
               log_d("S1 crossed after 3.5-5.5s and last Tstring was BAba. Invisible dog %i is running and next dog enters with fault.", iPreviousDog + 1);
            }
            else
            {
               if (_bRerunBusy)
               {
                  _llClearFaultTime = MICROS;
                  _bClearCurrentDogFault = true;
               }
               log_d("Dog %i going in crossed S1 safely. Clear fault if rerun.", iCurrentDog + 1);
            }
            _llDogEnterTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - _llLastDogExitTime;
            if (_llCrossingTimes[iCurrentDog] != 0)
            {
               LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
               WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
            }
            _bS1StillSafe = false;
         }
         else if (_byDogState == COMINGBACK && !_bS1StillSafe && (STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 2000000 // Filter out S1 HIGH signals that are < 2 seconds after dog enter time
                  && (iCurrentDog != iNextDog))                                                                                           // Exclude scenario if next dog is equal current dog as this can't be comming back dog.
         {
            _ChangeDogState(GOINGIN);
            _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llLastDogExitTime = _llDogExitTimes[iCurrentDog];
            _llDogTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llLastDogExitTime - _llDogEnterTimes[iCurrentDog];
            _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
            log_d("Dog %i EARLY as coming back dog was expected.", iNextDog + 1);
            if ((iCurrentDog == (iNumberOfRacingDogs - 1) && _bRerunNeeded && !_bRerunBusy && !bRerunsOff)
                  || _bRerunBusy)
            {

               _bRerunBusy = true;
               _llDogExitTimes[iNextDog] = 0;
               iDogRunCounters[iNextDog]++;
               log_d("Re-run for dog %i", iNextDog + 1);
            }
            else if (iNextDog == 5)
            {
               StopRace(STriggerRecord.llTriggerTime);
               log_i("Reruns off. No more dogs was expected. Race stopped.");
            }
            if (iNextDog != 5)
            {
               if (_bDogManualFaults[iCurrentDog])
               {
                  log_d("Potentialy dog %i comingback outside the gate. Delay fault activation for dog %i.", iCurrentDog + 1, iNextDog + 1);
                  _bPotentialyComingbackOutside = true;
               }
               else
               {
                  SetDogFault(iNextDog, ON);
                  LCDController.bUpdateThisLCDField[iNextDog + 4] = true;
                  LCDController.bUpdateThisLCDField[iNextDog + 8] = true;
#ifdef WiFiON
                  WebHandler.bUpdateThisRaceDataField[iNextDog] = true;
#endif
               }
            }
            _ChangeDogNumber(iNextDog);
         }
         else if (_byDogState == COMINGBACK && !_bS1StillSafe && (STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 2000000
                  && (iCurrentDog == iNextDog && _bDogManualFaults[iCurrentDog]))
         {
            _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llLastDogExitTime = _llDogExitTimes[iCurrentDog];
            _llDogTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llLastDogExitTime - _llDogEnterTimes[iCurrentDog];
            _bDogMissedGateComingback[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
            _bNoValidCrossingTime[iNextDog][iDogRunCounters[iNextDog] + 1] = true;
            _llDogEnterTimes[iNextDog] = STriggerRecord.llTriggerTime;
            _llClearFaultTime = MICROS;
            _bClearCurrentDogFault = true;
            LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
            LCDController.bUpdateThisLCDField[iCurrentDog + 8] = true;
#ifdef WiFiON
            WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
            iDogRunCounters[iNextDog]++;
            _bRerunBusy = true;
            _ChangeDogNumber(iNextDog);
         }
         else if (_byDogState == COMINGBACK && (_bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] || _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]])
                  && !_bS1StillSafe
                  && (((STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 100000 && (STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) < 2000000)
                     || (_bRerunBusy && iCurrentDog == iNextDog)))
         {
            if (_bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]])
            {
               _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]] = false;
               _bWasItBigOK = true;
            }
            else
               _bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] = false;

            _llDogEnterTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - _llLastDogExitTime;
            LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
            WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
            _bPrepareToRestoreokCrossing = true;
            log_d("False 'ok/OK crossing' detected. Recalculate dog %i times.", iCurrentDog + 1);
         }
      }
      //---------------------------------------------------------------------------------------------------------------------
      ////Handle SENSOR 1 events (handlers side) with gates state DOG IN
      if (STriggerRecord.iSensorNumber == 1 && STriggerRecord.iSensorState == 1 && !_bGatesClear && iCurrentDog < 5) // Only if gates are busy (dog in) and S1 sensor is HIGH (A)
      {
         if (_bPotentialNegativeCrossDetected && ((STriggerRecord.llTriggerTime - _llS2CrossedUnsafeTriggerTime) > 5679))
         {
            _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
            _bPotentialNegativeCrossDetected = false;
            _bNegativeCrossDetected = true;
            if (_bDogMissedGateComingback[iPreviousDog][iDogRunCounters[iPreviousDog]])
            {
               _bDogMissedGateComingback[iPreviousDog][iDogRunCounters[iPreviousDog]] = false;
               _bNoValidCrossingTime[iCurrentDog][iDogRunCounters[iCurrentDog]] = false;
               if (_bDogFaultOffAsPreviousDogMissedGateAssumed[iCurrentDog][iDogRunCounters[iCurrentDog]])
               {
                  SetDogFault(iCurrentDog, ON);
                  _bDogFaultOffAsPreviousDogMissedGateAssumed[iCurrentDog][iDogRunCounters[iCurrentDog]] = false;
                  log_d("Previous dog didn't missed the gate, so dog %i fault has been re-activated.", iCurrentDog + 1);
               }
            }
            if (!_bDogFaults[iCurrentDog] && iNextDog < 5)
            {
               _llDogEnterTimes[iNextDog] = _llDogEnterTimes[iCurrentDog];
               _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
               _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
               if (_bDogManualFaults[iCurrentDog])
               {
                  _bDogMissedGateGoingin[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                  _bNoValidCrossingTime[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               }
               else
                  _bDogInvisibleOk[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               if (_bRerunBusy)
               {
                  iDogRunCounters[iNextDog]++;
                  LCDController.bUpdateThisLCDField[iNextDog + 4] = true;
                  LCDController.bUpdateThisLCDField[iNextDog + 8] = true;
#ifdef WiFiON
                  WebHandler.bUpdateThisRaceDataField[iNextDog] = true;
#endif
               }
               log_d("Invisible dog %i came back! Dog times updated. Ok or Perfect crossing.", iNextDog + 1);
               SetDogFault(iNextDog, ON);
               LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
               WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
               _ChangeDogNumber(iNextDog);
            }
            _llDogExitTimes[iPreviousDog] = STriggerRecord.llTriggerTime;
            _llDogTimes[iPreviousDog][iDogRunCounters[iPreviousDog]] = _llDogExitTimes[iPreviousDog] - _llDogEnterTimes[iPreviousDog];
            _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogEnterTimes[iCurrentDog] - _llDogExitTimes[iPreviousDog];
            if (_bDogFakeTime[iPreviousDog][iDogRunCounters[iPreviousDog]])
            {
               _bDogFakeTime[iPreviousDog][iDogRunCounters[iPreviousDog]] = false;
               LCDController.bUpdateThisLCDField[iPreviousDog] = true;
#ifdef WiFiON
               WebHandler.bUpdateThisRaceDataField[iPreviousDog] = true;
#endif
               log_d("Fake time flag for dog %i cleared.", iPreviousDog + 1);
            }
            LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
            WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
            _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
            //
            log_d("Calculate negative cross time for dog %i and update times for previous dog %i.", iCurrentDog + 1, iPreviousDog + 1);
            log_d("Dod %i updated crossing time [ms]: %lld", iCurrentDog + 1, (_llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] + 500) / 1000);
            log_d("Dog %i updated time [ms]: %lld", iPreviousDog + 1, ((_llDogTimes[iPreviousDog][iDogRunCounters[iPreviousDog]] + 500) / 1000));
         }
         else if (_bS1isSafe)
         {
            _llRaceElapsedTime = STriggerRecord.llTriggerTime - llRaceStartTime;
            _llDogExitTimes[iCurrentDog] = STriggerRecord.llTriggerTime;
            _llLastDogExitTime = _llDogExitTimes[iCurrentDog];
            _llDogTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = _llDogExitTimes[iCurrentDog] - _llDogEnterTimes[iCurrentDog];
            _bS1isSafe = false;
            log_d("S1 line crossed while being safe. Calculate dog %i time.", iCurrentDog + 1);
            if (_llDogExitTimes[iCurrentDog] - _llS2CrossedSafeTime < 5000)
            {
               _bDogPerfectCross[iNextDog][iDogRunCounters[iNextDog]] = true;
               log_d("PERFECT cross below 5ms detected for dog %i.", iNextDog + 1);
            }
            if ((iCurrentDog == (iNumberOfRacingDogs - 1) && !_bRerunNeeded && !_bRerunBusy)
                  || (_bRerunBusy && !_bRerunNeeded)
                  || iNextDog == 5)
               StopRace(STriggerRecord.llTriggerTime);
            else if ((iCurrentDog == (iNumberOfRacingDogs - 1) && _bRerunNeeded && !_bRerunBusy)
                     || _bRerunBusy)
            {
               _bRerunBusy = true;
               iDogRunCounters[iNextDog]++;
               if (iNextDog != iCurrentDog)
                  _llDogEnterTimes[iNextDog] = _llDogExitTimes[iNextDog] = STriggerRecord.llTriggerTime;
               LCDController.bUpdateThisLCDField[iNextDog + 4] = true;
               LCDController.bUpdateThisLCDField[iNextDog + 8] = true;
#ifdef WiFiON
               WebHandler.bUpdateThisRaceDataField[iNextDog] = true;
#endif
               log_d("Re-run for dog %i", iNextDog + 1);
            }
            _ChangeDogNumber(iNextDog);
         }
      }
      //----------------------------------------------------------------------------------------------------------------------
      // Handle sensor 2 (box side) when GATE CLEAR
      if (STriggerRecord.iSensorNumber == 2 && STriggerRecord.iSensorState == 1 && _bGatesClear && iCurrentDog < 5) // Only if gates are clear S2 sensor is HIGH (B)
      {
         if (_byDogState == GOINGIN && iCurrentDog == 0 && !_bRerunBusy && !_bDogManualFaults[iCurrentDog] && ((STriggerRecord.llTriggerTime - (_llLastDogExitTime - 3000000)) < 10000000))
         {
            LightsController.DeleteSchedules();
            _llDogEnterTimes[iCurrentDog] = llRaceStartTime;
            _bWrongRunDirectionDetected = true;
            for (uint8_t i = 0; i < 4; i++)
               LightsController.ToggleFaultLight(i, LightsController.ON);
            LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
            WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
            log_d("Starting dog crossed S2 before S1. Wrong Run Direction detected!");
            StopRace(llRaceStartTime);
         }
         else if (_byDogState == GOINGIN && (STriggerRecord.llTriggerTime - _llLastDogExitTime) > 3000000)
         {
            if (iCurrentDog == 0 && !_bRerunBusy)
            {
               _llDogEnterTimes[iCurrentDog] = llRaceStartTime;
               _bDogMissedGateGoingin[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               _bNoValidCrossingTime[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               if (!_bDogManualFaults[iCurrentDog])
                  SetDogFault(iCurrentDog, ON);
               log_d("Invisible starting dog came back! Set fault and 'run in' as dog time.");
            }
            else
            {
               _llDogEnterTimes[iCurrentDog] = _llLastDogExitTime;
               _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
               if (_bDogManualFaults[iCurrentDog])
               {
                  _bDogMissedGateGoingin[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                  _bNoValidCrossingTime[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               }
               else
                  _bDogInvisibleOk[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
               log_d("Invisible dog %i came back!. Update enter time. Ok or Perfect crossing.", iCurrentDog + 1);
            }
            _bS1isSafe = true;
            _bS1StillSafe = true;
            _llS2CrossedSafeTime = STriggerRecord.llTriggerTime;
            _ChangeDogState(COMINGBACK);
            LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
#ifdef WiFiON
            WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
         }
         else if (_byDogState == COMINGBACK)
         {
            if (((STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) < 3000000) && (_bDogFaults[iCurrentDog] || _bDogManualFaults[iPreviousDog]))
            {
               _bPotentialNegativeCrossDetected = true;
               _llS2CrossedUnsafeTriggerTime = STriggerRecord.llTriggerTime;
               _llS2CrossedUnsafeGetMicrosTime = MICROS;
               log_d("Dog %i potential negative cross detected.", iCurrentDog + 1);
            }
            else if ((STriggerRecord.llTriggerTime - _llDogEnterTimes[iCurrentDog]) > 2000000)
            {
               _bS1isSafe = true;
               _bS1StillSafe = true;
               _llS2CrossedSafeTime = STriggerRecord.llTriggerTime;
               log_d("Coming back dog %i crossed S2 line. S1 is safe.", iCurrentDog + 1);
            }
         }
      }

      /***********************************
       * The code below handles what we call the 'transition string'
       * It is an algorithm which saves all sensor events in sequence, until it recognizes a pattern.
       * We have 2 sensor columns: the handler side column, and the box side column
       * To indicate the handler side column, we use the letter A
       * To indicate the box side column, we use the letter B
       * A HIGH sensor reading (beam broken), will be represented by an upper case character
       * A LOW sensor reading (beam not broken), will be represented by a lower case character
       * e.g.: A --> handler side HIGH reading, b --> box side LOW reading, etc...
       *
       * We chain these characters up to get our 'transition string'
       * Then we check this string to determine what happened
       * For example:
       * 'ABab'
       *    --> Handler side HIGH, box side HIGH, handler side LOW, box side LOW
       *    --> This tells us ONE dog passed the gates in the direction of the box
       * 'BAba'
       *    --> Box side HIGH, handler side HIGH, box side LOW, handler side LOW
       *    --> This tells us ONE dog passed the gates in the direction of the handler
       * 'BAab'
       *    --> Box side HIGH, handler side HIGH, handler side LOW, box side LOW
       *    --> This tells us TWO dogs crossed the gates simultaneously, and the dog going to the box was the last to leave the gates
       ***********************************/

      // Add trigger record to transition string
      _AddToTransitionString(STriggerRecord);

      String strLast2TransitionChars = _strTransition.substring(_strTransition.length() - 2);
      if (_strTransition.length() == 0
            || strLast2TransitionChars == "ab" || strLast2TransitionChars == "ba")
      {
         log_d("Tstring: %s", _strTransition.c_str());
         _bGatesClear = true;
         _bPrepareToRestoreokCrossing = false;
         _llGatesClearedTime = MICROS;
         log_d("Gate: CLEAR.");

         if (_strTransition.length() > 3)
         {
            if (_bLastStringBAba)
               _bLastStringBAba = false;
            if (_strTransition == "ABab")
            {
               _ChangeDogState(COMINGBACK);
               _strPreviousTransitionFirstLetter = "A";
               log_d("New dog state: COMINGBACK. ABab.");
               if (_bDogManualFaults[iPreviousDog] && _bPotentialyComingbackOutside)
               {
                  _bDogMissedGateComingback[iPreviousDog][iDogRunCounters[iPreviousDog]] = true;
                  _bNoValidCrossingTime[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                  _bDogFaultOffAsPreviousDogMissedGateAssumed[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                  _bPotentialyComingbackOutside = false;
                  _bDogDetectedFaults[iCurrentDog][iDogRunCounters[iCurrentDog]] = false;
                  LCDController.bUpdateThisLCDField[iPreviousDog] = true;
#ifdef WiFiON
                  WebHandler.bUpdateThisRaceDataField[iPreviousDog] = true;
#endif
                  log_d("Assumed previous dog missed the gate, so no fault for dog %i.", iCurrentDog + 1);
               }
            }
            else if (_strTransition == "BAba" && !_bNegativeCrossDetected && RaceState != STOPPED)
            {
               _ChangeDogState(GOINGIN);
               _strPreviousTransitionFirstLetter = "B";
               _bLastStringBAba = true;
               log_d("New dog state: GOINGING.");
            }
            else if (RaceState == STOPPED)
            {
               log_d("Last dog came back.");
               bIgnoreSensors = true;
            }
            else
            {
               String strFirstTransitionChar = _strTransition.substring(0, 1);
               if (_byDogState == COMINGBACK && strFirstTransitionChar == "B")
               {
                  _strPreviousTransitionFirstLetter = "B";
                  if (_bNegativeCrossDetected && RaceState != STOPPED)
                  {
                     _bNegativeCrossDetected = false;
                     log_d("Dog state still COMINGBACK. Dog coming back after negative cross of next dog.");
                  }
                  else if (!_bRaceStopRequested && RaceState == RUNNING && iCurrentDog != iPreviousDog)
                  {
                     if ((_bRerunBusy && _bRerunNeeded))
                     {
                        _llClearFaultTime = MICROS;
                        _bClearCurrentDogFault = true;
                     }
                     _bS1StillSafe = false;
                     _llCrossingTimes[iCurrentDog][iDogRunCounters[iCurrentDog]] = 0;
                     _llDogEnterTimes[iCurrentDog] = _llDogExitTimes[iPreviousDog];
                     if (_strTransition == "BAab" || _strTransition == "BAba" || _strTransition == "BbABab" || _strTransition == "BbABba")
                     {
                        _bDogBigOK[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                        log_d("Unmeasurable 'OK' crossing for dog %i.", iCurrentDog + 1);
                     }
                     else
                     {
                        _bDogSmallok[iCurrentDog][iDogRunCounters[iCurrentDog]] = true;
                        log_d("Unmeasurable 'ok' crossing for dog %i.", iCurrentDog + 1);
                     }
                     LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
                  #ifdef WiFiON
                     WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
                  #endif
                  }
                  else if (iCurrentDog == iPreviousDog)
                     _ChangeDogState(GOINGIN);
               }
               else if (_byDogState == COMINGBACK && strFirstTransitionChar == "A")
               {
                  _strPreviousTransitionFirstLetter = "A";
                  log_d("Dog %i fault. Tstring starting with A.", iCurrentDog + 1);
               }
               else
               {
                  _ChangeDogState(COMINGBACK);
                  _strPreviousTransitionFirstLetter = _strTransition.substring(0, 1);
                  log_d("New dog %i state: COMINGBACK. Uncertain.", iCurrentDog + 1);
               }
            }
            if (_bPotentialyComingbackOutside)
            {
               _bPotentialyComingbackOutside = false;
               log_d("Previous dog didn't missed the gate so activating fault for dog %i. iPreviousDog is %i.", iCurrentDog + 1, iPreviousDog + 1);
               SetDogFault(iCurrentDog, ON, iPreviousDog);
               LCDController.bUpdateThisLCDField[iCurrentDog + 4] = true;
               LCDController.bUpdateThisLCDField[iCurrentDog + 8] = true;
#ifdef WiFiON
               WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
            }
         }
         _strTransition = "";
      }
      else if (_bGatesClear)
      {
         _bGatesClear = false;
         log_d("Gate: DOG(s)");
      }
   }

   _bRerunNeeded = false;
   for (uint8_t i = 0; i < iNumberOfRacingDogs; i++)
   {
      if (_bDogFaults[i] || _bDogManualFaults[i])
      {
         _bRerunNeeded = true;
         if (!_bNoValidCleanTime)
         {
            _bNoValidCleanTime = true;
            LCDController.bUpdateThisLCDField[LCDController.CleanTime] = true;
#ifdef WiFiON
            WebHandler.bUpdateThisRaceDataField[WebHandler.cleanTime] = true;
#endif
         }
         break;
      }
   }

   if (!bIgnoreSensors && RaceState == STOPPED && (MICROS - _llRaceEndTime) > 400000)
      bIgnoreSensors = true;

   if (bIgnoreSensors && !_bRaceSummaryPrinted)
   {
      _PrintRaceSummary();
      _bRaceSummaryPrinted = true;
   }
}

/// <summary>
///   Clear transistion string and Gate state.
/// </summary>
///
void RaceHandlerClass::_ClearTransitionString()
{
   _strTransition = "";
   _bGatesClear = true;
   log_d("Reset transition strings. Gate: CLEAR.");
}

/// <summary>
///   Changes race state, if byNewRaceState is different from current one.
/// </summary>
///
/// <param name="byNewRaceState">   New race state. </param>
void RaceHandlerClass::_ChangeRaceState(RaceStates byNewRaceState)
{
   RaceState = byNewRaceState;
   switch (RaceState)
   {
   case RaceHandlerClass::RESET:
      strRaceState = " READY ";
      break;
   case RaceHandlerClass::STARTING:
      strRaceState = " START ";
      break;
   case RaceHandlerClass::RUNNING:
      strRaceState = "RUNNING";
      break;
   case RaceHandlerClass::STOPPED:
      strRaceState = " STOP  ";
      break;
   default:
      break;
   }
   log_i("RS: %s", strRaceState);
   LCDController.UpdateField(LCDController.RaceState, strRaceState);
#ifdef WiFiON
   if (RaceState != 0)
   {
      WebHandler.bUpdateThisRaceDataField[WebHandler.raceState] = true;
      WebHandler.bSendRaceData = true;
   }
#endif
}

/// <summary>
///   Change dog state. If byNewDogState is different from current one.
/// </summary>
///
/// <param name="byNewDogState"> State of the new dog. </param>
void RaceHandlerClass::_ChangeDogState(_byDogStates byNewDogState)
{
   if (_byDogState != byNewDogState)
      _byDogState = byNewDogState;
}

/// <summary>
///   Change dog number, if new dognumber is different from current one.
/// </summary>
///
/// <param name="iNewDogNumber"> Zero-based index of the new dog number. </param>
void RaceHandlerClass::_ChangeDogNumber(uint8_t iNewDogNumber)
{
   LCDController.bUpdateThisLCDField[iCurrentDog] = true;
#ifdef WiFiON
   WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
   iPreviousDog = iCurrentDog;
   iCurrentDog = iNewDogNumber;
   log_d("Dog:%i|ENT:%lld|EXIT:%lld|TOT:%lld", iPreviousDog + 1, _llDogEnterTimes[iPreviousDog], _llLastDogExitTime, _llDogTimes[iPreviousDog][iDogRunCounters[iPreviousDog]]);
   if (!_bNoValidCleanTime)
   {
      LCDController.bUpdateThisLCDField[LCDController.CleanTime] = true;
#ifdef WiFiON
      WebHandler.bUpdateThisRaceDataField[WebHandler.cleanTime] = true;
#endif
   }
   if (RaceState == RUNNING)
   {
#ifdef WiFiON
      WebHandler.bUpdateThisRaceDataField[iPreviousDog + 8] = true;
      WebHandler.bUpdateThisRaceDataField[iCurrentDog + 8] = true;
#endif
      log_i("Dog %i: %s | CR: %s", iPreviousDog + 1, GetDogTime(iPreviousDog, iDogRunCounters[iPreviousDog]), GetCrossingTime(iPreviousDog, iDogRunCounters[iPreviousDog]).c_str());
      log_d("Running dog: %i.", iCurrentDog + 1);
   }
}

/// <summary>
///   Initiates race start timer and sets the status of the race to STARTING.
///   Should be called when RED light comes ON during start sequence.
/// </summary>
void RaceHandlerClass::StartRaceTimer()
{
   llRaceStartTime = _llLastDogExitTime = MICROS + 3000000;
   _ChangeRaceState(STARTING);
   log_i("STARTING! Tag: %i, Race ID: %i.", SDcardController.iTagValue, iCurrentRaceId + 1);
   cRaceStartTimestamp = GPSHandler.GetLocalTimestamp();
   log_i("Timestamp: %s", cRaceStartTimestamp);
}

/// <summary>
///   Stops a race.
/// </summary>
/// <param name="StopTime">   The time in microseconds at which the race stopped. </param>
void RaceHandlerClass::StopRace(long long llStopTime)
{
   if (RaceState == STOPPED || RaceState == RESET)
      return;
   else
   {
      // Race is running, so we have to record the EndTime
      _llRaceEndTime = llStopTime;
      if (RaceState == RUNNING)
         _llRaceTime = _llRaceEndTime - llRaceStartTime;
      else
         _llRaceTime = 0;
      LCDController.bUpdateThisLCDField[LCDController.TeamTime] = true;
      LCDController.bUpdateThisLCDField[iCurrentDog] = true;
#ifdef WiFiON
      WebHandler.bUpdateThisRaceDataField[WebHandler.elapsedTime] = true;
      WebHandler.bUpdateThisRaceDataField[iCurrentDog] = true;
#endif
      if (!_bNoValidCleanTime)
      {
         LCDController.bUpdateThisLCDField[LCDController.CleanTime] = true;
#ifdef WiFiON
         WebHandler.bUpdateThisRaceDataField[WebHandler.cleanTime] = true;
#endif
      }
      if (bRaceStoppedManually || _llRaceTime == 0)
         _ChangeRaceState(STOPPED);
      else
         _bRaceStopRequested = true;
      //
   }
}

/// <summary>
///   Resets the race, this function should be called to reset all timers to 0 and prepare the
///   software for starting a next race.
/// </summary>
void RaceHandlerClass::ResetRace()
{
   if (RaceState != STOPPED)
      return;
   else
   {
      NOW = MICROS;
      iCurrentDog = 0;
      iNextDog = 1;
      iPreviousDog = 0;
      llRaceStartTime = NOW;
      _llRaceEndTime = NOW;
      _llGatesClearedTime = NOW;
      _llRaceTime = 0;
      _llRaceElapsedTime = 0;
      _llLastDogExitTime = NOW;
      _llS2CrossedSafeTime = NOW;
      _llS2CrossedUnsafeTriggerTime = NOW;
      _llS2CrossedUnsafeGetMicrosTime = NOW;
      _llClearFaultTime = NOW;
      _byDogState = GOINGIN;
      _ChangeDogNumber(0);
#if Simulate
      Simulator.bExecuteSimRaceReset = true;
#endif
      _bRerunNeeded = false;
      _bRerunBusy = false;
      _iOutputQueueReadIndex = 0;
      _iInputQueueReadIndex = 0;
      _iOutputQueueWriteIndex = 0;
      _iInputQueueWriteIndex = 0;
      _strTransition = "";
      _strPreviousTransitionFirstLetter = "";
      _bGatesClear = true;
      _bS1isSafe = false;
      _bS1StillSafe = false;
      _bNextDogFound = false;
      _bNegativeCrossDetected = false;
      _bPotentialNegativeCrossDetected = false;
      _bPotentialyComingbackOutside = false;
      _bLastStringBAba = false;
      _bNoValidCleanTime = false;
      _bWrongRunDirectionDetected = false;
      _bPrepareToRestoreokCrossing = false;
      _bWasItBigOK = false;
      _bClearCurrentDogFault = false;
      for (auto &bFault : _bDogFaults)
         bFault = false;
      for (auto &bManualFault : _bDogManualFaults)
         bManualFault = false;
      for (auto &Dog : _bDogDetectedFaults)
      {
         for (auto &bDogDetectedFaults : Dog)
            bDogDetectedFaults = false;
      }
      for (auto &Dog : _bDogFaultOffAsPreviousDogMissedGateAssumed)
      {
         for (auto &bDogFaultOffAsPreviousDogMissedGateAssumed : Dog)
            bDogFaultOffAsPreviousDogMissedGateAssumed = false;
      }
      for (auto &Dog : _bDogDetectedManualFaults)
      {
         for (auto &bDogDetectedManualFaults : Dog)
            bDogDetectedManualFaults = false;
      }
      for (auto &Dog : _bDogPerfectCross)
      {
         for (auto &bDogPerfectCross : Dog)
            bDogPerfectCross = false;
      }
      for (auto &Dog : _bDogBigOK)
      {
         for (auto &bDogBigOK : Dog)
            bDogBigOK = false;
      }
      for (auto &Dog : _bDogSmallok)
      {
         for (auto &bDogSmallok : Dog)
            bDogSmallok = false;
      }
      for (auto &Dog : _bDogInvisibleOk)
      {
         for (auto &bDogInvisibleOk : Dog)
            bDogInvisibleOk = false;
      }
      for (auto &Dog : _bDogFakeTime)
      {
         for (auto &bDogFakeTime : Dog)
            bDogFakeTime = false;
      }
      for (auto &Dog : _bDogMissedGateGoingin)
      {
         for (auto &bDogMissedGateGoingin : Dog)
            bDogMissedGateGoingin = false;
      }
      for (auto &Dog : _bDogMissedGateComingback)
      {
         for (auto &bDogMissedGateComingback : Dog)
            bDogMissedGateComingback = false;
      }
      for (auto &Dog : _bNoValidCrossingTime)
      {
         for (auto &bNoValidCrossingTime : Dog)
            bNoValidCrossingTime = false;
      }
      for (auto &llTime : _llDogEnterTimes)
         llTime = 0;
      for (auto &llTime : _llDogExitTimes)
         llTime = 0;
      for (auto &Dog : _llDogTimes)
      {
         for (auto &llTime : Dog)
            llTime = 0;
      }
      for (auto &Dog : _llCrossingTimes)
      {
         for (auto &llTime : Dog)
            llTime = 0;
      }
      for (auto &iCounter : iDogRunCounters)
         iCounter = 0;
      for (auto &llTime : _llLastDogTimeReturnTimeStamp)
         llTime = 0;
      for (auto &iCounter : _iLastReturnedRunNumber)
         iCounter = 0;
      _ChangeRaceState(RESET);
      bIgnoreSensors = false;
      bRaceStoppedManually = false;
      _bRaceSummaryPrinted = false;

      if (iCurrentRaceId == 998)
      {
         iCurrentRaceId = 0;
         SDcardController.iTagValue = SDcardController.iTagValue + 1;
      }
      else
         iCurrentRaceId++;
      String _sCurrentRaceId = String(iCurrentRaceId + 1);
      while (_sCurrentRaceId.length() < 3)
         _sCurrentRaceId = " " + _sCurrentRaceId;
      LCDController.UpdateField(LCDController.RaceID, _sCurrentRaceId);
      LCDController.bUpdateTimerLCDdata = true;
      _strManualFaultsRecords = "// $commands;";
      _strRaceManualStopTime = "";
      log_i("Reset Race: DONE");
#ifdef WiFiON
      WebHandler.bUpdateRaceData = true;
      WebHandler.bSendRaceData = true;
#endif
   }
}

void RaceHandlerClass::_PrintRaceSummary()
{
   log_v("RaceState: %i, MICROS: %lld, _llRaceEndTime: %lld, Delta: %lld, _bRaceSummaryPrinted: %i", RaceState, MICROS, _llRaceEndTime, MICROS - _llRaceEndTime, _bRaceSummaryPrinted);
   for (uint8_t i = 0; i < iNumberOfRacingDogs; i++)
   {
      for (uint8_t i2 = 0; i2 < (iDogRunCounters[i] + 1); i2++)
         log_i("Dog %i: %s | CR: %s", i + 1, GetStoredDogTimes(i, i2), TransformCrossingTime(i, i2));
   }
   log_i(" Team: %s", GetRaceTime());
   log_i("   CT: %s", GetCleanTime());
   if (SDcardController.bSDCardDetected)
   {
      SDcardController.SaveRaceDataToFile();
      _PrintRaceTriggerRecordsToFile();
   }
   if (CORE_DEBUG_LEVEL >= ESP_LOG_VERBOSE)
      _PrintRaceTriggerRecords();
}

/// <summary>
///   After race is ended/stopped print trigger records to console
/// </summary>
void RaceHandlerClass::_PrintRaceTriggerRecords()
{
   uint8_t iRecordToPrintIndex = 0;
   while (iRecordToPrintIndex < _iInputQueueWriteIndex)
   {
      STriggerRecord RecordToPrint = _InputTriggerQueue[iRecordToPrintIndex];
      printf("{%i, %lld, %i},\r\n", RecordToPrint.iSensorNumber, RecordToPrint.llTriggerTime - llRaceStartTime, RecordToPrint.iSensorState);
      iRecordToPrintIndex++;
   }
}

/// <summary>
///   If SD card is present, after race is ended/stopped print trigger records to file
/// </summary>
void RaceHandlerClass::_PrintRaceTriggerRecordsToFile()
{
   File rawSensorsReadingFile;
   String rawSensorsReadingFileName = "/SENSORS_DATA/" + SDcardController.sTagValue + "_SensorsData" + ".txt";
   if (iCurrentRaceId == 0)
   {
      SDcardController.writeFile(SD_MMC, rawSensorsReadingFileName.c_str(),
                                 "ID; Time [us]; state\n");
   }
   rawSensorsReadingFile = SD_MMC.open(rawSensorsReadingFileName.c_str(), FILE_APPEND);
   if (rawSensorsReadingFile)
   {
      rawSensorsReadingFile.print("Race ID: ");
      rawSensorsReadingFile.println(RaceHandler.iCurrentRaceId + 1);
      uint8_t iRecordToPrintIndex = 0;
      rawSensorsReadingFile.print("// $init;setdogs ");
      rawSensorsReadingFile.print(iNumberOfRacingDogs);
      rawSensorsReadingFile.print(";reruns ");
      if (bRerunsOff)
         rawSensorsReadingFile.print("off");
      else
         rawSensorsReadingFile.print("on");
      rawSensorsReadingFile.println(";");
      if (_strManualFaultsRecords.length() > 13)
         rawSensorsReadingFile.println(_strManualFaultsRecords.c_str());
      if (bRaceStoppedManually)
      {
         rawSensorsReadingFile.print("// Manual Race STOP time: ");
         rawSensorsReadingFile.println(_strRaceManualStopTime.c_str());
      }
      while (iRecordToPrintIndex < _iInputQueueWriteIndex)
      {
         STriggerRecord RecordToPrint = _InputTriggerQueue[iRecordToPrintIndex];
         rawSensorsReadingFile.print("{");
         rawSensorsReadingFile.print(RecordToPrint.iSensorNumber);
         rawSensorsReadingFile.print(", ");
         rawSensorsReadingFile.print(RecordToPrint.llTriggerTime - llRaceStartTime);
         rawSensorsReadingFile.print(", ");
         rawSensorsReadingFile.print(RecordToPrint.iSensorState);
         rawSensorsReadingFile.println("},");
         iRecordToPrintIndex++;
      }
      rawSensorsReadingFile.print("// Number of records: ");
      rawSensorsReadingFile.println(iRecordToPrintIndex);
      rawSensorsReadingFile.close();
   }
}

/// <summary>
///   Sets dog fault for given dog number to given state.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="State">      The state. </param>
void RaceHandlerClass::SetDogFault(uint8_t iDogNumber, DogFaults State, int8_t iPreviousDogNumber)
{
   // Don't process any faults when race is not running
   if (RaceState == STOPPED || RaceState == RESET)
      return;
   if (iPreviousDogNumber == -1)
      iPreviousDogNumber = iCurrentDog;
   bool bFault;
   bool bCalculateManualFaultTimestamp = false;
   if (State == TOGGLE)
   {
      bFault = !_bDogManualFaults[iDogNumber];
      _bDogManualFaults[iDogNumber] = bFault;
      if (bFault)
      {
         _bDogDetectedManualFaults[iDogNumber][iDogRunCounters[iDogNumber]] = true;
         bCalculateManualFaultTimestamp = true;
         if (iCurrentDog == iNextDog && !_bRerunBusy)
            _bRerunBusy = true;
         log_i("Manual change of dog %i fault to ON.", iDogNumber + 1);
      }
      else
      {
         _bDogDetectedManualFaults[iDogNumber][iDogRunCounters[iDogNumber]] = false;
         bCalculateManualFaultTimestamp = true;
         log_i("Manual change of dog %i fault to OFF.", iDogNumber + 1);
      }
   }
   else if (State == ON)
   {
      bFault = true;
      _bDogFaults[iDogNumber] = true;
      _bDogDetectedFaults[iDogNumber][iDogRunCounters[iDogNumber]] = true;
      if ((iDogNumber > 0 || (iDogNumber == 0 && iCurrentDog > 0)) && !_bDogFaultOffAsPreviousDogMissedGateAssumed[iCurrentDog][iDogRunCounters[iCurrentDog]])
      {
         _bDogFakeTime[iPreviousDogNumber][iDogRunCounters[iPreviousDogNumber]] = true;
         LCDController.bUpdateThisLCDField[iPreviousDogNumber] = true;
#ifdef WiFiON
         WebHandler.bUpdateThisRaceDataField[iPreviousDogNumber] = true;
#endif
      }
   }
   else if (State == OFF)
   {
      bFault = false;
      if (_bDogFaults[iDogNumber])
         _bDogFaults[iDogNumber] = false;
      if (_bDogManualFaults[iDogNumber])
         _bDogManualFaults[iDogNumber] = false;
   }

   if (bFault)
   {
      LightsController.ToggleFaultLight(iDogNumber, LightsController.ON);
      log_i("Dog %i fault ON. Current Dog: %i, Next Dog: %i, Dog Run Counter: %i", iDogNumber + 1, iCurrentDog + 1, iNextDog + 1, iDogRunCounters[iDogNumber]);
   }
   else
   {
      LightsController.ToggleFaultLight(iDogNumber, LightsController.OFF);
      log_i("Dog %i fault OFF", iDogNumber + 1);
   }
   LCDController.bUpdateThisLCDField[iDogNumber + 4] = true;
#ifdef WiFiON
   WebHandler.bUpdateThisRaceDataField[iDogNumber + 4] = true;
#endif

   if (bCalculateManualFaultTimestamp)
   {
      bCalculateManualFaultTimestamp = false;
      long long llManualFaultTimestamp = MICROS - llRaceStartTime;
      double dManualFaultTime = ((long long)(llManualFaultTimestamp + 50000) / 100000) / 10.0;
      std::string strManualFaultTimestamp = std::to_string(dManualFaultTime);
      strManualFaultTimestamp = strManualFaultTimestamp.substr(0, strManualFaultTimestamp.find(".") + 2);
      _strManualFaultsRecords += strManualFaultTimestamp + ";d" + std::to_string(iDogNumber + 1) + "f;";
   }
}

/// <summary>
///   ISR function for sensor 1, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void IRAM_ATTR RaceHandlerClass::TriggerSensor1(portMUX_TYPE *spinlock)
{
   if (bIgnoreSensors)
      return;
   else if (RaceState == RESET)
   {
      if (digitalRead(_iS1Pin) == 1)
         LightsController.bS1ExecuteRaceReadyFaultON = true;
      else
         LightsController.bS1ExecuteRaceReadyFaultOFF = true;
   }
   else
   {
      taskENTER_CRITICAL_ISR(spinlock);
      _QueuePush({bRunDirectionInverted ? 2 : 1, MICROS, digitalRead(_iS1Pin)});
      taskEXIT_CRITICAL_ISR(spinlock);
   }
}

/// <summary>
///   ISR function for sensor 2, this function will record the sensor number, microseconds and
///   state (HIGH/LOW) of the sensor in the interrupt queue.
/// </summary>
void IRAM_ATTR RaceHandlerClass::TriggerSensor2(portMUX_TYPE *spinlock)
{
   if (bIgnoreSensors)
      return;
   else if (RaceState == RESET)
   {
      if (digitalRead(_iS2Pin) == 1)
         LightsController.bS2ExecuteRaceReadyFaultON = true;
      else
         LightsController.bS2ExecuteRaceReadyFaultOFF = true;
   }
   else
   {
      taskENTER_CRITICAL_ISR(spinlock);
      _QueuePush({bRunDirectionInverted ? 1 : 2, MICROS, digitalRead(_iS2Pin)});
      taskEXIT_CRITICAL_ISR(spinlock);
   }
}

/// <summary>
///   Gets race time. Time since start if race is still running, final time if race is finished.
/// </summary>
///
/// <returns>
///   The race time in seconds with milisecond accuracy rounded up or down.
/// </returns>
String RaceHandlerClass::GetRaceTime()
{
   char cRaceTimeSeconds[8];
   String strRaceTimeSeconds;
   double dRaceTimeSeconds;
   if (!_bAccuracy3digits)
   {
      dRaceTimeSeconds = ((long long)(_llRaceTime + 5000) / 10000) / 100.0;
      dtostrf(dRaceTimeSeconds, 7, 2, cRaceTimeSeconds);
   }
   else
   {
      dRaceTimeSeconds = ((long long)(_llRaceTime + 500) / 1000) / 1000.0;
      dtostrf(dRaceTimeSeconds, 7, 3, cRaceTimeSeconds);
   }
   strRaceTimeSeconds = cRaceTimeSeconds;
   if ((bRaceStoppedManually && !_bRaceStopRequested) || _bWrongRunDirectionDetected)
   {
      _strRaceManualStopTime = strRaceTimeSeconds;
      strRaceTimeSeconds = "     nt";
   }
   return strRaceTimeSeconds;
}

/// <summary>
///   Gets team Clean Time in seconds.
///   In case of heat would have a fault(s) Clean Time will not be displayed as it has no meaning.
///   In such situation "nt" will be shown.
///   Please mark that "Cleat Time" term is often use in the meaning of Clean Time Breakout.
///   Please refer to FCI Regulations for Flyball Competition section 1.03 point (h).
/// </summary>
String RaceHandlerClass::GetCleanTime()
{
   String strCleanTime;
   if (!_bNoValidCleanTime && !_bWrongRunDirectionDetected)
   {
      long long llTotalNetTime = 0;
      for (auto &Dog : _llDogTimes)
      {
         for (auto &llNetTime : Dog)
         {
            if (llNetTime > 0)
               llTotalNetTime += llNetTime;
         }
      }
      char cCleanTime[8];
      double dCleanTime;
      if (!_bAccuracy3digits)
      {
         dCleanTime = ((long long)(llTotalNetTime + 5000) / 10000) / 100.0;
         dtostrf(dCleanTime, 7, 2, cCleanTime);
      }
      else
      {
         dCleanTime = ((long long)(llTotalNetTime + 500) / 1000) / 1000.0;
         dtostrf(dCleanTime, 7, 3, cCleanTime);
      }
      strCleanTime = cCleanTime;
   }
   else
      strCleanTime = "     nt";
   return strCleanTime;
}

/// <summary>
///   Gets dog time, time since dog entered if dog is still running, final time if dog is already
///   back in. Keep in mind each dog can have multiple runs (reruns for faults).
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   String with following options:
///   - dog time if available
///   - 0.000 or 0.00 if no dog time available
///   - partial time if dog is still running
///   - " run in" if manual fault active and "invisible dog" detected (assumed gate was missed while entering)
///   - "outside" if manual fault active and ABab string detected for next dog (assumed gate was missed while exiting)
/// </returns>

String RaceHandlerClass::GetDogTime(uint8_t iDogNumber, int8_t iRunNumber)
{
   char cDogTime[8];
   String strDogTime;
   double dDogTime = 0;
   unsigned long ulDogTimeMillis = 0;
   if (_llDogTimes[iDogNumber][iRunNumber] > 0)
      ulDogTimeMillis = (_llDogTimes[iDogNumber][iRunNumber] + 500) / 1000;
   else if ((RaceState == RUNNING && iCurrentDog == iDogNumber && _byDogState == COMINGBACK) && iRunNumber <= iDogRunCounters[iDogNumber] //
            && _llDogEnterTimes[iDogNumber] != 0)
      ulDogTimeMillis = (MICROS - _llDogEnterTimes[iDogNumber]) / 1000;
   else if (_llDogTimes[iDogNumber][iRunNumber] == 0)
      ulDogTimeMillis = 0;

   if (!_bAccuracy3digits)
   {
      dDogTime = ((unsigned long)(ulDogTimeMillis + 5) / 10) / 100.0;
      dtostrf(dDogTime, 7, 2, cDogTime);
   }
   else
   {
      dDogTime = ulDogTimeMillis / 1000.0;
      dtostrf(dDogTime, 7, 3, cDogTime);
   }

   if (_bWrongRunDirectionDetected && iDogNumber == 0 && iRunNumber == 0)
      strDogTime = " <-  ->";
   else if (_bDogMissedGateGoingin[iDogNumber][iRunNumber])
      strDogTime = " run in";
   else if (_bDogMissedGateComingback[iDogNumber][iRunNumber])
      strDogTime = "outside";
   else if (dDogTime == 0 && RaceState != RESET)
      strDogTime = "       ";
   else
   {
      strDogTime = cDogTime;
      if (_bDogFakeTime[iDogNumber][iRunNumber] && (!_bAccuracy3digits || (_bAccuracy3digits && dDogTime < 100)))
         strDogTime[0] = '#';
   }
   return strDogTime;
}

/// <summary>
///   Gets stored dog time for specific run.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. </param>
///
/// <returns>
///   The dog time in seconds with miliseconds accuracy and rounded up or down.
/// </returns>
String RaceHandlerClass::GetStoredDogTimes(uint8_t iDogNumber, int8_t iRunNumber)
{
   char cDogTime[8];
   String strDogTime;
   double dDogTime = 0;
   if (!_bAccuracy3digits)
   {
      dDogTime = ((long long)(_llDogTimes[iDogNumber][iRunNumber] + 5000) / 10000) / 100.0;
      dtostrf(dDogTime, 7, 2, cDogTime);
   }
   else
   {
      dDogTime = ((long long)(_llDogTimes[iDogNumber][iRunNumber] + 500) / 1000) / 1000.0;
      dtostrf(dDogTime, 7, 3, cDogTime);
   }
   if (_bWrongRunDirectionDetected && iDogNumber == 0 && iRunNumber == 0)
      strDogTime = " <-  ->";
   else if (_bDogMissedGateGoingin[iDogNumber][iRunNumber])
      strDogTime = " run in";
   else if (_bDogMissedGateComingback[iDogNumber][iRunNumber])
      strDogTime = "outside";
   else if (dDogTime == 0 && RaceState != RESET)
      strDogTime = "       ";
   else
   {
      strDogTime = cDogTime;
      if (_bDogFakeTime[iDogNumber][iRunNumber])
         strDogTime[0] = '#';
   }

   return strDogTime;
}

/// <summary>
///   Gets dogs crossing time. Keep in mind each dog can have multiple runs (reruns for faults).
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   The dog time in seconds with miliseconds accuracy rounded up or down.
/// </returns>
String RaceHandlerClass::GetCrossingTime(uint8_t iDogNumber, int8_t iRunNumber)
{
   String strCrossingTime;
   strCrossingTime = TransformCrossingTime(iDogNumber, iRunNumber);
   return strCrossingTime;
}

/// <summary>
///   Transform crossing time to string.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. </param>
/// <param name="bToFile"> used for results printing to file. </param>
///
/// <returns>
///   String with following options:
///   * positive cross value rounded up or down
///   * ok - good un-measurable cross
///   * Perfect - perfect cross with S1 crossed below 5ms after S2 have been crossed
///   * OK - good un-measurable cross with string BAab
///   * Ok - good un-measurable cross with invisible dog run in secnario (BAba)
///   * negative cross value - measurable fault rounded up or down
///   * fault - un-measurable fault
///   * "empty" - if no crossing time available / possible
/// </returns>
String RaceHandlerClass::TransformCrossingTime(uint8_t iDogNumber, int8_t iRunNumber, bool bToFile)
{
   double dCrossingTime;
   char cCrossingTime[8];
   String strCrossingTime;
   if (_llCrossingTimes[iDogNumber][iRunNumber] < 0 && _llDogEnterTimes[iDogNumber] != 0)
   {
      if ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] > -9500) && !_bAccuracy3digits && !bToFile)
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] - 500) / 1000);
         dCrossingTime = fabs(dCrossingTime);
         dtostrf(dCrossingTime, 3, 0, cCrossingTime);
         strCrossingTime = "-";
         strCrossingTime += cCrossingTime;
         strCrossingTime += " ms";
      }
      else if (_bAccuracy3digits || ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] > -9500) && !_bAccuracy3digits && bToFile))
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] - 500) / 1000) / 1000.0;
         dCrossingTime = fabs(dCrossingTime);
         dtostrf(dCrossingTime, 7, 3, cCrossingTime);
         strCrossingTime = "-";
         strCrossingTime += cCrossingTime;
      }
      else
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] - 5000) / 10000) / 100.0;
         dCrossingTime = fabs(dCrossingTime);
         dtostrf(dCrossingTime, 6, 2, cCrossingTime);
         strCrossingTime = "-";
         strCrossingTime += cCrossingTime;
      }
   }
   else if (_llCrossingTimes[iDogNumber][iRunNumber] > 0 && _llDogEnterTimes[iDogNumber] != 0)
   {
      if ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] < 9500) && !_bAccuracy3digits && !bToFile)
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] + 500) / 1000);
         dtostrf(dCrossingTime, 3, 0, cCrossingTime);
         strCrossingTime = "+";
         strCrossingTime += cCrossingTime;
         strCrossingTime += " ms";
      }
      else if (_bAccuracy3digits || ((iDogNumber == 0 && iRunNumber == 0 && _llCrossingTimes[0][0] < 9500) && !_bAccuracy3digits && bToFile))
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] + 500) / 1000) / 1000.0;
         dtostrf(dCrossingTime, 7, 3, cCrossingTime);
         strCrossingTime = "+";
         strCrossingTime += cCrossingTime;
      }
      else
      {
         dCrossingTime = ((long long)(_llCrossingTimes[iDogNumber][iRunNumber] + 5000) / 10000) / 100.0;
         dtostrf(dCrossingTime, 6, 2, cCrossingTime);
         strCrossingTime = "+";
         strCrossingTime += cCrossingTime;
      }
   }
   else if (_bWrongRunDirectionDetected && iDogNumber == 0 && iRunNumber == 0)
      strCrossingTime = "  ERROR";
   else if (_bDogPerfectCross[iDogNumber][iRunNumber])
      strCrossingTime = "Perfect";
   else if (_bDogInvisibleOk[iDogNumber][iRunNumber])
      strCrossingTime = "     Ok";
   else if (_bDogBigOK[iDogNumber][iRunNumber])
      strCrossingTime = "     OK";
   else if (_bDogSmallok[iDogNumber][iRunNumber])
      strCrossingTime = "     ok";
   else if (_bNoValidCrossingTime[iDogNumber][iRunNumber])
      strCrossingTime = "     nt";
   else if (_bDogDetectedFaults[iDogNumber][iRunNumber])
      strCrossingTime = "  early";
   else
      strCrossingTime = " ";

   if (_bDogDetectedManualFaults[iDogNumber][iRunNumber])
   {
      if (_llCrossingTimes[iDogNumber][iRunNumber] > 0)
         strCrossingTime.replace("+ ", "F+");
      else if (_llCrossingTimes[iDogNumber][iRunNumber] < 0)
         strCrossingTime.replace("- ", "F-");
      else if (strCrossingTime == "Perfect")
         strCrossingTime = "F Perfe";
      else if (_bNoValidCrossingTime[iDogNumber][iRunNumber])
         strCrossingTime = "F    nt";
      else if (_llCrossingTimes[iDogNumber][iRunNumber] == 0 && strCrossingTime.length() < 7)
         strCrossingTime = "F early";
      else
         strCrossingTime[0] = 'F';
   }
   if (_bAccuracy3digits && strCrossingTime.length() == 7)
      strCrossingTime = " " + strCrossingTime;

   return strCrossingTime;
}

/// <summary>
///   Gets rerun information to put on LCD. If a dog has done more than 1 run, before the dogs
///   time we will display and * (asterisk) followed by the run number for which we are showing
///   the time. If a dog has not done a rerun, the space before the dogs time is empty.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
///
/// <returns>
///   The rerun information. * (asterisk) followed by run number if there is more than 1 run for
///   this dog. Empty string if the dog did only do 1 run.
/// </returns>
String RaceHandlerClass::GetRerunInfo(uint8_t iDogNumber, int8_t iRunNumber)
{
   String strRerunInfo = "  ";
   if (bRerunsOff)
      strRerunInfo = "*X";
   else if (iDogRunCounters[iDogNumber] > 0)
   {
      strRerunInfo = "*";
      strRerunInfo += (iRunNumber + 1);
   }
   return strRerunInfo;
}

/// <summary>
///   Supporting alternate function of multiple dog results.
/// </summary>
///
/// <param name="iDogNumber"> Zero-based index of the dog number. </param>
/// <param name="iRunNumber"> Zero-based index of the run number. If -1 is passed, this function
///                           will alternate between each run we have for the dog, passing a new
///                           run every 2 seconds. If -2 is passed, the last run number we have
///                           for this dog will be passed. </param>
///
/// <returns>
///   Transformed iRunNumber.
/// </returns>
int8_t RaceHandlerClass::SelectRunNumber(uint8_t iDogNumber, int8_t iRunNumber)
{
   if (iDogRunCounters[iDogNumber] > 0)
   {
      if (iRunNumber == -1 && ((iDogNumber != iCurrentDog) || (iDogNumber == iCurrentDog && RaceState == STOPPED)))
      {
         auto &ulLastReturnedTimeStamp = _llLastDogTimeReturnTimeStamp[iDogNumber];
         iRunNumber = _iLastReturnedRunNumber[iDogNumber];
         if ((millis() - ulLastReturnedTimeStamp) > 2000)
         {
            if (iRunNumber == iDogRunCounters[iDogNumber])
               iRunNumber = 0;
            else
               iRunNumber++;
            _llLastDogTimeReturnTimeStamp[iDogNumber] = millis();
         }
         _iLastReturnedRunNumber[iDogNumber] = iRunNumber;
      }
      else
         iRunNumber = iDogRunCounters[iDogNumber];
   }
   else if (iRunNumber < 0)
      iRunNumber = 0;
   return iRunNumber;
}

/// <summary>
///   Toggles the direction the system expects dogs to run in
/// </summary>
void RaceHandlerClass::ToggleRunDirection()
{
   if (RaceState == STOPPED || RaceState == RESET)
   {
      bRunDirectionInverted = !bRunDirectionInverted;
      SettingsManager.setSetting("RunDirectionInverted", String(bRunDirectionInverted));
      if (bRunDirectionInverted)
      {
         LCDController.UpdateField(LCDController.BoxDirection, "<");
         log_i("Run direction changed to: inverted");
      }
      else
      {
         LCDController.UpdateField(LCDController.BoxDirection, ">");
         log_i("Run direction changed to: normal");
      }
      LCDController.bExecuteLCDUpdate = true;
#ifdef WiFiON
      WebHandler.bSendRaceData = true;
#endif
   }
   else
      return;
}

/// <summary>
///   Toggles displayed results accuracy between 2 and 3 digits
/// </summary>
void RaceHandlerClass::ToggleAccuracy()
{
   _bAccuracy3digits = !_bAccuracy3digits;
   SettingsManager.setSetting("Accuracy3digits", String(_bAccuracy3digits));
   LCDController.bUpdateTimerLCDdata = true;
   LCDController.bExecuteLCDUpdate = true;
   if (_bAccuracy3digits)
      log_i("Accuracy switched to 3 digits");
   else
      log_i("Accuracy switched to 2 digits");
#ifdef WiFiON
   WebHandler.bUpdateRaceData = true;
   WebHandler.bSendRaceData = true;
#endif
}

/// <summary>
///   Toggles status or reruns off/on
/// </summary>
void RaceHandlerClass::ToggleRerunsOffOn(uint8_t _iState)
{
   if (RaceState == STOPPED || RaceState == RESET)
   {
      if (_iState == 2)
         bRerunsOff = !bRerunsOff;
      else if (_iState == 1)
         bRerunsOff = true;
      else if (_iState == 0)
         bRerunsOff = false;
      LCDController.bUpdateThisLCDField[LCDController.D1RerunInfo] = true;
      LCDController.bUpdateThisLCDField[LCDController.D2RerunInfo] = true;
      LCDController.bUpdateThisLCDField[LCDController.D3RerunInfo] = true;
      LCDController.bUpdateThisLCDField[LCDController.D4RerunInfo] = true;
      LCDController.bExecuteLCDUpdate = true;
      if (bRerunsOff)
         log_i("Reruns turned off.");
      else
         log_i("Reruns turned on.");
#ifdef WiFiON
      WebHandler.bUpdateThisRaceDataField[WebHandler.rerunsOff] = true;
      WebHandler.bSendRaceData = true;
#endif
   }
   else
      return;
}

/// <summary>
///   Set number of racing dogs
/// </summary>
void RaceHandlerClass::SetNumberOfDogs(uint8_t _iNumberOfRacingDogs)
{
   iNumberOfRacingDogs = _iNumberOfRacingDogs;
   LCDController.UpdateNumberOfDogsOnLCD(iNumberOfRacingDogs);
#ifdef WiFiON
   WebHandler.bUpdateRaceData = true;
   WebHandler.bSendRaceData = true;
#endif
   log_i("Number of dogs set to: %i.", iNumberOfRacingDogs);
}

/// <summary>
///   Pushes an Sensor interrupt trigger record to the back of the input interrupts buffer.
/// </summary>
///
/// <param name="_InterruptTrigger">   The interrupt trigger record. </param>
void RaceHandlerClass::_QueuePush(RaceHandlerClass::STriggerRecord _InterruptTrigger)
{
   // Add record to queue
   _InputTriggerQueue[_iInputQueueWriteIndex] = _InterruptTrigger;

   if (_iInputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1)
      _iInputQueueWriteIndex = 0;
   else
      _iInputQueueWriteIndex++;
}

/// <summary>
///   Filter input interrupt record(s) from the front of the interrupts buffer
///   and push filtered records to output interrupts records queue
/// </summary>
void RaceHandlerClass::_QueueFilter()
{
   STriggerRecord _PreviousRecord = _InputTriggerQueue[_iInputQueueReadIndex - 1];
   STriggerRecord _CurrentRecord = _InputTriggerQueue[_iInputQueueReadIndex];
   STriggerRecord _NextRecord = _InputTriggerQueue[_iInputQueueReadIndex + 1];
   STriggerRecord _2ndNextRecord = _InputTriggerQueue[_iInputQueueReadIndex + 2];

   if ((_iInputQueueReadIndex <= _iInputQueueWriteIndex - 2) && (_CurrentRecord.llTriggerTime - _PreviousRecord.llTriggerTime <= 200000) //
      && (_CurrentRecord.iSensorNumber == _NextRecord.iSensorNumber && _NextRecord.llTriggerTime - _CurrentRecord.llTriggerTime <= 5679))
   {
      log_d("S%i | TT:%lld | T:%lld | St:%i | IGNORED-1", _CurrentRecord.iSensorNumber, _CurrentRecord.llTriggerTime,
            _CurrentRecord.llTriggerTime - llRaceStartTime, _CurrentRecord.iSensorState);
      log_d("S%i | TT:%lld | T:%lld | St:%i | IGNORED-2", _NextRecord.iSensorNumber, _NextRecord.llTriggerTime,
            _NextRecord.llTriggerTime - llRaceStartTime, _NextRecord.iSensorState);

      if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 2)
         _iInputQueueReadIndex = 0;
      else
         _iInputQueueReadIndex = _iInputQueueReadIndex + 2;
   }
   else if ((_iInputQueueReadIndex <= _iInputQueueWriteIndex - 3) //
         && (_CurrentRecord.llTriggerTime - _PreviousRecord.llTriggerTime <= 200000) //
         && (_CurrentRecord.iSensorNumber == _2ndNextRecord.iSensorNumber //
            && _2ndNextRecord.llTriggerTime - _CurrentRecord.llTriggerTime <= 5679))
   {
      log_d("S%i | TT:%lld | T:%lld | St:%i | IGNORED-2nd-1", _CurrentRecord.iSensorNumber, _CurrentRecord.llTriggerTime,
            _CurrentRecord.llTriggerTime - llRaceStartTime, _CurrentRecord.iSensorState);
      log_d("S%i | TT:%lld | T:%lld | St:%i | IGNORED-2nd-2", _2ndNextRecord.iSensorNumber, _2ndNextRecord.llTriggerTime,
            _2ndNextRecord.llTriggerTime - llRaceStartTime, _2ndNextRecord.iSensorState);

      _OutputTriggerQueue[_iOutputQueueWriteIndex] = _NextRecord;
      if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 3)
         _iInputQueueReadIndex = 0;
      else
         _iInputQueueReadIndex = _iInputQueueReadIndex + 3;

      if (_iOutputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1)
         _iOutputQueueWriteIndex = 0;
      else
         _iOutputQueueWriteIndex++;
   }
   else
   {
      _OutputTriggerQueue[_iOutputQueueWriteIndex] = _InputTriggerQueue[_iInputQueueReadIndex];
      if (_iInputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 1)
         _iInputQueueReadIndex = 0;
      else
         _iInputQueueReadIndex++;

      if (_iOutputQueueWriteIndex == TRIGGER_QUEUE_LENGTH - 1)
         _iOutputQueueWriteIndex = 0;
      else
         _iOutputQueueWriteIndex++;
   }
}

/// <summary>
///   Pops one interrupt record from the front of the interrupt buffer.
/// </summary>
///
/// <returns>
///   A RaceHandlerClass::STriggerRecord from the front of the interrupt buffer.
/// </returns>
RaceHandlerClass::STriggerRecord RaceHandlerClass::_QueuePop()
{
   STriggerRecord NextRecord = _OutputTriggerQueue[_iOutputQueueReadIndex];

   if (_iOutputQueueReadIndex == TRIGGER_QUEUE_LENGTH - 1)
      _iOutputQueueReadIndex = 0;
   else
      _iOutputQueueReadIndex++;
   return NextRecord;
}

/// <summary>
///   Determines if the interrupt buffer queue is empty.
/// </summary>
///
/// <returns>
///   true if it is empty, false if it is not.
/// </returns>
bool RaceHandlerClass::_QueueEmpty()
{
   if (_iOutputQueueReadIndex == _iOutputQueueWriteIndex)
      return true;
   else
      return false;
}

/// <summary>
///   Adds an interrupt record to the transition string. This function will automatically
///   determine which character (upper or lowercase A or B) should be added to the string. Note
///   that some filtering of consecutive LOW-HIGH-LOW and HIGH-LOW-HIGH signals is done also in
///   this function.
/// </summary>
///
/// <param name="_InterruptTrigger">   The interrupt trigger record. </param>
void RaceHandlerClass::_AddToTransitionString(STriggerRecord _InterruptTrigger)
{
   _llLastTransitionStringUpdate = MICROS;

   char cTemp;
   switch (_InterruptTrigger.iSensorNumber)
   {
   case 1:
      cTemp = 'A';
      break;
   case 2:
      cTemp = 'B';
      break;
   default:
      cTemp = 'X';
      break;
   }

   if (_InterruptTrigger.iSensorState == LOW)
      cTemp = tolower(cTemp);
   _strTransition += cTemp;

   if (_strTransition.endsWith("AaA"))
   {
      _strTransition.replace("AaA", "A");
      log_d("Tstring AaA replaced with A");
   }
   if (_strTransition.endsWith("aAa"))
   {
      _strTransition.replace("aAa", "a");
      log_d("Tstring aAa replaced with a");
   }
   if (_strTransition.endsWith("BbB"))
   {
      _strTransition.replace("BbB", "B");
      log_d("Tstring BbB replaced with B");
   }
   if (_strTransition.endsWith("bBb"))
   {
      _strTransition.replace("bBb", "b");
      log_d("Tstring bBb replaced with b");
   }
}

RaceHandlerClass RaceHandler;
