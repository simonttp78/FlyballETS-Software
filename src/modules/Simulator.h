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
      56, 34, 26, 24, 24, 40, 46, 38, 44, 48
   };
};

extern class SimulatorClass Simulator;

#endif