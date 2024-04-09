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
   {
      56, 34, 26, 24, 24, 40, 46, 38, 44, 48,
      46, 54, 32, 40, 40, 46, 40, 38, 24, 34,
      32, 52, 56, 46, 34, 38, 36, 32, 30, 44,
      48, 42, 42, 32, 44, 38, 46, 50, 44, 32,
      34, 26, 26, 58, 28, 52, 48, 28, 40,  6,
       6, 54, 56, 52, 52
   };
};

extern class SimulatorClass Simulator;

#endif