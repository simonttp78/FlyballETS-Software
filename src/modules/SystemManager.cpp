#include <SystemManager.h>

void SystemManagerClass::init()
{
    _ulScheduledRebootTS = 0;
    _iOpMode = SettingsManager.getSetting("OperationMode").toInt();
    if (_iOpMode == 1)
        log_i("ETS in DUAL operation mode RED");
    else if (_iOpMode == 2)
        log_i("ETS in DUAL operation mode BLUE");
    else
        log_i("ETS in SINGLE operation mode");
}

void SystemManagerClass::loop()
{
    if (_ulScheduledRebootTS > 0 && millis() > _ulScheduledRebootTS)
        ESP.restart();
}

void SystemManagerClass::scheduleReboot(unsigned long ulRebootTs)
{
    if (ulRebootTs > millis())
        _ulScheduledRebootTS = ulRebootTs;
}

bool SystemManagerClass::CheckRedWithBlueConnection()
{
    if (_iOpMode == SystemModes::RED)
        return BlueNodeHandler.GetConnectionStatus();
    else if (_iOpMode == SystemModes::BLUE)
        return WebHandler.RedNodeConnected();
    else
        return false;
}

/// <summary>
///   Toggles operation mode of ETS
/// </summary>
void SystemManagerClass::ToggleOpMode()
{
   if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
   {
        if (_iOpMode < 2)
            _iOpMode = _iOpMode + 1;
        else
            _iOpMode = 0;
        SettingsManager.setSetting("OperationMode", String(_iOpMode));
        if (_iOpMode == 0)
        {
            log_i("ETS operation mode switched to SINGLE");
            scheduleReboot(millis() + 2000);
        }   
        else if (_iOpMode == 1)
        {
            log_i("ETS operation mode switched to DUAL RED");
            scheduleReboot(millis() + 2000);
        }  
        else if (_iOpMode == 2)
        {
            log_i("ETS operation mode switched to DUAL BLUE");
            scheduleReboot(millis() + 2000);
        }  
   }

}

SystemManagerClass SystemManager;