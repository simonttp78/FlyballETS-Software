#include "SettingsManager.h"
#include "Embedis.h"
#include <EEPROM.h>
#include <SPIFFS.h>
#include "config.h"

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
       []() {});

   setDefaultSettings();
}

String SettingsManagerClass::getSetting(const String &key, String defaultValue)
{
   String value;
   if (!Embedis::get(key, value))
      value = defaultValue;
   return value;
}

String SettingsManagerClass::getSetting(const String &key)
{
   return getSetting(key, "");
}

// template<typename T> bool SettingsManagerClass::setSetting(const String& key, T value)
bool SettingsManagerClass::setSetting(const String &key, String value)
{
   saveSettings();
   return Embedis::set(key, String(value));
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
   if (!hasSetting("AdminPass"))
   {
      setSetting("AdminPass", "FlyballETS.1234");
      saveSettings();
   }

   if (!hasSetting("APName"))
   {
      setSetting("APName", "FlyballETS");
      saveSettings();
   }

   if (!hasSetting("APPass"))
   {
      setSetting("APPass", "FlyballETS.1234");
      saveSettings();
   }

   if (!hasSetting("RunDirectionInverted"))
   {
      setSetting("RunDirectionInverted", String("0"));
      saveSettings();
   }

   if (!hasSetting("StartingSequenceNAFA"))
   {
      setSetting("StartingSequenceNAFA", String("0"));
      saveSettings();
   }

   if (!hasSetting("LaserOnTimer"))
   {
      setSetting("LaserOnTimer", String("60"));
      saveSettings();
   }

   if (!hasSetting("Accuracy3digits"))
   {
      setSetting("Accuracy3digits", String("0"));
      saveSettings();
   }

   if (!hasSetting("CommaInCsv"))
   {
      setSetting("CommaInCsv", String("0"));
      saveSettings();
   }

   if (!hasSetting("OperationMode"))
   {
      setSetting("OperationMode", String("0"));
      saveSettings();
   }
}

SettingsManagerClass SettingsManager;
