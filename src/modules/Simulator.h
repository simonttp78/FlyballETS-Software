// Simulator.h
#ifndef _SIMULATOR_h
#define _SIMULATOR_h

#include "config.h"
#include "Arduino.h"

class SimulatorClass
{
protected:
public:
   void init();
   void Main();
   uint iSimulatedRaceID = 0;
   bool bExecuteSimRaceReset = false;
   bool bExecuteSimRaceChange = false;

private:
   uint _iDataPos;
   uint _iDataStartPos;
   uint _iDataEndPos;
   bool _bNoMoreValidTriggers = false;
   typedef struct SimulatorRecord
   {
      uint8_t iSimSensorNumber;
      long long llSimTriggerTime;
      uint8_t iSimState;
   } SimulatorRecord;
   static const SimulatorRecord SimulatorQueue[] PROGMEM;
   SimulatorRecord PendingRecord;
   uint calculateDataStartPos(uint iSimulatedRaceID);
   
   // array of number of records per simulated race
   u_int8_t _iNumberOfRecordsInSimulatedRace[NumSimulatedRaces] =
   {//x0, x1, x2, x3, x4, x5, x6, x7, x8, x9
      56, 34, 26, 24, 24, 40, 46, 38, 44, 48, // 0x
      46, 54, 32, 40, 40, 46, 40, 38, 24, 34, // 1x
      32, 52, 56, 46, 34, 38, 36, 32, 30, 44, // 2x
      48, 42, 42, 32, 44, 38, 46, 50, 44, 32, // 3x
      34, 26, 26, 58, 28, 52, 48, 28, 40,  6, // 4x
       6, 54, 56, 52, 56, 50, 60, 64, 46, 46  // 5x
   };
};

extern class SimulatorClass Simulator;

#endif