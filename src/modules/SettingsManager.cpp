#include "SettingsManager.h"
#include "Embedis.h"
#include <EEPROM.h>
#include <SPIFFS.h>
#include "config.h"


void SettingsManagerClass::init()
{
   EEPROM.begin(EEPROM_SIZE);
   _settings_save = false;

   Embedis::dictionary(
       F("EEPROM"),
       SPI_FLASH_SEC_SIZE,
       [](size_t pos) -> char
       { return EEPROM.read(pos); },
       [](size_t pos, char value)
       { EEPROM.write(pos, value); },
       []() { EEPROM.commit(); });

   setDefaultSettings();
}

void SettingsManagerClass::loop()
{
   // do we really need to check in loop if there is something to write???
   if (_settings_save)
   {
      log_i("[SETTINGS] Saving to EEPROM");
      EEPROM.commit();
      _settings_save = false;
   }
}

String SettingsManagerClass::getSetting(const String &key, String defaultValue)
{
   String value;
   if (!Embedis::get(key, value))
      value = defaultValue;
   //log_d("Returning value %s for setting %s", value.c_str(), key.c_str());
   return value;
}

String SettingsManagerClass::getSetting(const String &key)
{
   String strReturnValue = getSetting(key, "");
   return strReturnValue;
}

void SettingsManagerClass::setSetting(const String &key, String value)
{
   //log_d("Setting value %s for key %s", value.c_str(), key.c_str());
   Embedis::set(key, String(value));
   saveSettings();
}

void SettingsManagerClass::saveSettings()
{
   _settings_save = true;
   // log_d("Save settings flag set");
}

bool SettingsManagerClass::hasSetting(const String &key)
{
   return getSetting(key).length() != 0;
}

void SettingsManagerClass::setDefaultSettings()
{
   bool shouldSave = false;
   if (!hasSetting("AdminPass"))
   {
      setSetting("AdminPass", "FlyballETS.1234");
      shouldSave = true;
   }

   if (!hasSetting("APName"))
   {
      setSetting("APName", "FlyballETS");
      shouldSave = true;
   }

   if (!hasSetting("APPass"))
   {
      setSetting("APPass", "FlyballETS.1234");
      shouldSave = true;
   }

   if (!hasSetting("RunDirectionInverted"))
   {
      setSetting("RunDirectionInverted", String("0"));
      shouldSave = true;
   }

   if (!hasSetting("StartingSequenceNAFA"))
   {
      setSetting("StartingSequenceNAFA", String("0"));
      shouldSave = true;
   }

   if (!hasSetting("LaserOnTimer"))
   {
      setSetting("LaserOnTimer", String("60"));
      shouldSave = true;
   }

   if (!hasSetting("Accuracy3digits"))
   {
      setSetting("Accuracy3digits", String("0"));
      shouldSave = true;
   }

   if (!hasSetting("CommaInCsv"))
   {
      setSetting("CommaInCsv", String("0"));
      shouldSave = true;
   }

   if (!hasSetting("OperationMode"))
   {
      setSetting("OperationMode", String("0"));
      shouldSave = true;
   }
   if (shouldSave)
      saveSettings();
}

SettingsManagerClass SettingsManager;
