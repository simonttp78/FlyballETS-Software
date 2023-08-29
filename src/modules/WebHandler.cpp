#include "WebHandler.h"
#include <AsyncElegantOTA.h>

void WebHandlerClass::init(int webPort)
{

   // Populate the last modification date based on build datetime
   // sprintf(_last_modified, "%s %s GMT", __DATE__, __TIME__);
   snprintf_P(_last_modified, sizeof(_last_modified), PSTR("%s %s GMT"), __DATE__, __TIME__);
   _bBlueNodePresent = false;
   _server = new AsyncWebServer(webPort);
   _ws = new AsyncWebSocket("/ws");
   _wsa = new AsyncWebSocket("/wsa");

   _lWebSocketReceivedTime = 0;

   _ws->onEvent(std::bind(&WebHandlerClass::_WsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
   _wsa->onEvent(std::bind(&WebHandlerClass::_WsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

   _server->addHandler(_ws);
   _server->addHandler(_wsa);

   _server->onNotFound([](AsyncWebServerRequest *request)
                       {
      log_e("Not found: %s!", request->url().c_str());
      request->send(404); });

   // Rewrites
   _server->rewrite("/", "/index.html");
#ifdef WebUIonSDcard
   _server->serveStatic("/", SD_MMC, "/");
#endif

   // Serve home (basic authentication protection)
   _server->on("/index.html", HTTP_GET, std::bind(&WebHandlerClass::_onHome, this, std::placeholders::_1));

   // Authentication handler
   _server->on("/auth", HTTP_GET, std::bind(&WebHandlerClass::_onAuth, this, std::placeholders::_1));

   // Favicon handler
   _server->on("/favicon.ico", HTTP_GET, std::bind(&WebHandlerClass::_onFavicon, this, std::placeholders::_1));

   String password = SettingsManager.getSetting("AdminPass");
   char httpPassword[password.length() + 1];
   password.toCharArray(httpPassword, password.length() + 1);
   AsyncElegantOTA.begin(_server, "Admin", httpPassword); // Start ElegantOTA

   _server->begin();

   _lLastRaceDataBroadcast = 0;
   _lLastSystemDataBroadcast = 0;
   _lLastPingBroadcast = 0;
   _lLastBroadcast = 0;

   _iPwrOnTag = SDcardController.iTagValue;

   for (bool &bIsConsumer : _bIsConsumerArray)
      bIsConsumer = false;
   _iNumOfConsumers = 0;
}

void WebHandlerClass::loop()
{
   unsigned long lCurrentUpTime = millis();
   if ((lCurrentUpTime - _lLastRaceDataBroadcast > _iRaceDataBroadcastInterval) && !bSendRaceData && (RaceHandler.RaceState == RaceHandler.STARTING || RaceHandler.RaceState == RaceHandler.RUNNING))
      bSendRaceData = true;
   // log_d("bSendRaceData: %i, bUpdateLights: %i, since LastBroadcast: %ul, since WS received: %ul", bSendRaceData, bUpdateLights, (lCurrentUpTime - _lLastBroadcast), (lCurrentUpTime - _lWebSocketReceivedTime));
   if ((lCurrentUpTime - _lLastBroadcast > 100) && (lCurrentUpTime - _lWebSocketReceivedTime > 50))
   {
      if (bUpdateLights)
         _SendLightsData();
      else if (bSendRaceData)
         _SendRaceData(RaceHandler.iCurrentRaceId, -1);
      else if (RaceHandler.RaceState == RaceHandler.RESET || (RaceHandler.RaceState == RaceHandler.STOPPED && RaceHandler.bIgnoreSensors))
      {
         if (lCurrentUpTime - _lLastSystemDataBroadcast > _iSystemDataBroadcastInterval)
            _SendSystemData();
         if (lCurrentUpTime - _lLastPingBroadcast > _iPingBroadcastInterval)
         {
            //_ws->pingAll();
            _ws->cleanupClients();
            log_d("Have %i clients, %i consumers", _ws->count(), _iNumOfConsumers);
            //_lLastBroadcast = millis();
            _lLastPingBroadcast = millis();
         }
      }
   }
   _CheckRedNodeStatus();
}

void WebHandlerClass::_WsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
   bool isAdmin = String("/wsa").equals(server->url());

   if (type == WS_EVT_CONNECT)
   {
      if (client->remoteIP().toString().equals("192.168.20.1"))
      {
         //This client is the master!
         _RedNodeStatus.client = _ws->client(client->id());
         _RedNodeStatus.ClientID = client->id();
         _RedNodeStatus.ip = client->remoteIP();
         _RedNodeStatus.Configured = true;
         _RedNodeStatus.LastReply = millis();
         //_RedNodeStatus.LastCheck = millis();
      }
      log_i("Client %i connected to %s!", client->id(), server->url());

      // Access via /wsa. Checking if clinet/consumer has already athenticated
      if (isAdmin)
      {
         if (!_wsAuth(client))
         {
            client->close(401, "Unauthorized");
            return;
         }
      }
      // client->ping();
      // client->keepAlivePeriod(10);
   }
   else if (type == WS_EVT_DISCONNECT)
   {
      if (client->id() == _RedNodeStatus.ClientID)
      {
         log_i("Disconnecting master due to disconnect event from ws\r\n");
         _DisconnectRedNode();
      }

      if (_bIsConsumerArray[client->id()])
      {
         _iNumOfConsumers--;
         _bIsConsumerArray[client->id()] = false;
      }

      log_i("Client %u disconnected! Have %i clients", client->id(), _ws->count());
   }
   else if (type == WS_EVT_ERROR)
   {
      log_e("ws[%s][%u] error(%u): %s", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
   }
   else if (type == WS_EVT_PONG)
   {
      if (client->id() == _RedNodeStatus.ClientID)
      {
         _RedNodeStatus.LastReply = millis();
         log_d("Pong received from master (%lu)!\r\n", millis());
      }
      log_d("ws[%s][%u] pong[%u]: %s", server->url(), client->id(), len, (len) ? (char *)data : "");
   }
   else if (type == WS_EVT_DATA)
   {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      String msg = "";
      if (info->final && info->index == 0 && info->len == len)
      {
         // the whole message is in a single frame and we got all of it's data
         // log_d("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

         if (info->opcode == WS_TEXT)
         {
            data[len] = 0;
            msg = (char *)data;
         }
         else
         {
            char buff[3];
            for (size_t i = 0; i < info->len; i++)
            {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         log_d("%s", msg.c_str());
      }
      else
      {
         // message is comprised of multiple frames or the frame is split into multiple packets
         if (info->index == 0)
         {
            if (info->num == 0)
               log_d("ws[%s][%u] %s-message start", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
            log_d("ws[%s][%u] frame[%u] start[%llu]", server->url(), client->id(), info->num, info->len);
         }

         log_d("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

         if (info->opcode == WS_TEXT)
         {
            data[len] = 0;
            msg = (char *)data;
         }
         else
         {
            char buff[3];
            for (size_t i = 0; i < info->len; i++)
            {
               sprintf(buff, "%02x ", (uint8_t)data[i]);
               msg += buff;
            }
         }
         log_d("%s", msg.c_str());

         if ((info->index + len) == info->len)
         {
            log_d("ws[%s][%u] frame[%u] end[%llu]", server->url(), client->id(), info->num, info->len);
            if (info->final)
               log_d("ws[%s][%u] %s-message end", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
         }
      }

      // Parse JSON input
      StaticJsonDocument<768> jsonRequestDoc;
      DeserializationError error = deserializeJson(jsonRequestDoc, msg);
      JsonObject request = jsonRequestDoc.as<JsonObject>();
      if (error)
      {
         log_e("Error parsing JSON: %s", error.c_str());
         client->text("{\"error\":\"Invalid JSON received\"}");
         return;
      }

      _lWebSocketReceivedTime = millis();
      // const size_t bufferSize = JSON_ARRAY_SIZE(50) + 50 * JSON_OBJECT_SIZE(3);
      const size_t bufferSize = 384;
      StaticJsonDocument<bufferSize> jsonResponseDoc;
      JsonObject JsonResponseRoot = jsonResponseDoc.to<JsonObject>();

      if (request.containsKey("action"))
      {
         JsonObject ActionResult = JsonResponseRoot.createNestedObject("ActionResult");
         String errorText;
         bool result = _DoAction(request["action"].as<JsonObject>(), &errorText, client);
         ActionResult["success"] = result;
         ActionResult["error"] = errorText;
      }
      else if (request.containsKey("config"))
      {
         JsonObject ConfigResult = JsonResponseRoot.createNestedObject("configResult");
         String errorText;
         JsonArray config = request["config"].as<JsonArray>();
         // We allow setting config only over admin websocket
         bool result;
         if (isAdmin)
         {
            result = _ProcessConfig(config, &errorText);
         }
         else
         {
            result = false;
            errorText = "Unsupported message on this socket!";
         }
         ConfigResult["success"] = result;
         ConfigResult["error"] = errorText;
      }
      else if (request.containsKey("getData"))
      {
         String dataName = request["getData"];
         JsonObject DataResult = JsonResponseRoot.createNestedObject("dataResult");
         JsonObject DataObject = DataResult.createNestedObject(dataName + "Data");
         bool result;
         if (dataName == "config" && !isAdmin)
         {
            result = false;
            DataResult["error"] = "You can't get this datatype on this socket!";
         }
         else
         {
            result = _GetData(dataName, DataObject);
         }
         DataResult["success"] = result;
      }
      else
      {
         log_w("Got valid JSON but unknown message!");
         JsonResponseRoot["error"] = "Got valid JSON but unknown message!";
      }

      size_t len = measureJson(jsonResponseDoc);
      std::shared_ptr<std::vector<uint8_t>> buffer;
      buffer = std::make_shared<std::vector<uint8_t>>(len);
      _lLastBroadcast = millis();
      if (std::move(buffer))
      {
         serializeJson(jsonResponseDoc, (char *)buffer->data(),len);
         //std::string strBufferToPrint = (char *)buffer->data();
         //log_d("Respnse buffer to send: %s", (strBufferToPrint.erase (strBufferToPrint.find ("xV"), strBufferToPrint [strBufferToPrint.length() - 2])).c_str());
         client->text(std::move(buffer));
      }
   }
}

bool WebHandlerClass::_DoAction(JsonObject ActionObj, String *ReturnError, AsyncWebSocketClient *Client)
{
   String ActionType = ActionObj["actionType"];
   if (ActionType == "UpdateRace")
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
      {
         bUpdateRaceData = true;
         bSendRaceData = true;
         bUpdateLights = true;
         return true;
      }
      else
      {
         return false;
      }
   }
   if (ActionType == "StartRace")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
         // ReturnError = String("Race was not reset, stop and reset it first!");
         return false;
      else
      {
         if (_bBlueNodePresent)
         {
            unsigned long ulStartTime = GPSHandler.GetEpochTime() + 2;

            StaticJsonDocument<bsActionScheduleStartRace> jsonDoc;

            JsonObject root = jsonDoc.to<JsonObject>();

            JsonObject action = root.createNestedObject("action");
            action["actionType"] = "ScheduleStartRace";
            JsonObject action_actionData = action.createNestedObject("actionData");
            action_actionData["startTime"] = ulStartTime;

            String strScheduleStartMessage;
            serializeJson(root, strScheduleStartMessage);
            BlueNodeHandler.sendToBlueNode(strScheduleStartMessage);

            long lMillisToStart = GPSHandler.GetMillisToEpochSecond(ulStartTime);
            if (lMillisToStart < 0)
               return false;
            
            if (LightsController.bModeNAFA)
               LightsController.WarningStartSequence(lMillisToStart + millis());
            else
               LightsController.InitiateStartSequence(lMillisToStart + millis());
         }
         else
         {
            if (LightsController.bModeNAFA)
               LightsController.WarningStartSequence();
            else
               LightsController.InitiateStartSequence();
         }
         return true;
      }
   }
   else if (ActionType == "StopRace")
   {
      if (RaceHandler.RaceState == RaceHandler.STOPPED || RaceHandler.RaceState == RaceHandler.RESET)
      {
         // ReturnError = "Race was already stopped!";
         bUpdateTimerWebUIdata = true;
         bSendRaceData = true;
         bUpdateLights = true;
         return false;
      }
      else
      {
         LightsController.DeleteSchedules();
         RaceHandler.bExecuteStopRace = true;
         if (_bBlueNodePresent)
            BlueNodeHandler.sendToBlueNode("{ \"action\": {\"actionType\": \"StopRace\"}}");
         return true;
      }
   }
   else if (ActionType == "ResetRace")
   {
      if (RaceHandler.RaceState != RaceHandler.STOPPED)
      {
         // ReturnError = "Race was not stopped, or already in RESET state.";
         bUpdateTimerWebUIdata = true;
         bSendRaceData = true;
         bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.bExecuteResetRace = true;
         LightsController.bExecuteResetLights = true;
         if (_bBlueNodePresent)
            BlueNodeHandler.sendToBlueNode("{ \"action\": {\"actionType\": \"ResetRace\"}}");
         return true;
      }
   }
   else if (ActionType == "SetDogFault")
   {
      if (!ActionObj.containsKey("actionData"))
      {
         // ReturnError = "No actionData found!";
         return false;
      }
      uint8_t iDogNum = ActionObj["actionData"]["dogNr"];
      // bool bFaultState = ActionObj["actionData"]["faultState"];
      // RaceHandler.SetDogFault(iDogNum, (bFaultState ? RaceHandler.ON : RaceHandler.OFF));
      RaceHandler.SetDogFault(iDogNum);
      return true;
   }
   else if (ActionType == "ScheduleStartRace") //ScheduleStartRace action is used to schedule a race start on 2 lanes. This command would typically be sent by the master to the slave
   {
      //Check if race is in reset state. Instead of returning error if it isn't, we just stop and reset it.
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         LightsController.DeleteSchedules();
         RaceHandler.bExecuteStopRace = true;
         RaceHandler.bExecuteResetRace = true;
         LightsController.bExecuteResetLights = true;
      }

      String StartTime = ActionObj["actionData"]["startTime"];

      unsigned long lStartEpochTime = StartTime.toInt();
      log_d("StartTime S: %s, UL: %lu\r\n", StartTime.c_str(), lStartEpochTime);
      long lMillisToStart = GPSHandler.GetMillisToEpochSecond(lStartEpochTime);

      log_i("Received request to schedule race to start at %lu s which is in %ld ms", lStartEpochTime, lMillisToStart);

      if (lMillisToStart < 0)
      {
         //ReturnError = "Requested starttime is in the past!";
         log_i("Race schedule received for the past (%ld ms)!", lMillisToStart);
         return false;
      }

      if (LightsController.bModeNAFA)
         LightsController.WarningStartSequence(lMillisToStart + millis());
      else
         LightsController.InitiateStartSequence(lMillisToStart + millis());
      return true;
   }
   else if (ActionType == "AnnounceBlue")
   {
      log_i("We have Blue ETS with IP %s", Client->remoteIP().toString().c_str());
      BlueNodeHandler.configureBlueNode(Client->remoteIP());
      _bBlueNodePresent = true;
      return true;
   }
   else if (ActionType == "AnnounceConsumer")
   {
      log_d("We have a consumer with ID %i and IP %s", Client->id(), Client->remoteIP().toString().c_str());
      if (!_bIsConsumerArray[Client->id()])
         _iNumOfConsumers++;
      _bIsConsumerArray[Client->id()] = true;
      bUpdateRaceData = true;
      bSendRaceData = true;
      bUpdateLights = true;
      return true;
   }
   else if (ActionType == "SetDogs4")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         bSendRaceData = true;
         bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(4);
         return true;
      }
   }
   else if (ActionType == "SetDogs3")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         bSendRaceData = true;
         bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(3);
         return true;
      }
   }
   else if (ActionType == "SetDogs2")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         bSendRaceData = true;
         bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(2);
         return true;
      }
   }
   else if (ActionType == "SetDogs1")
   {
      if (RaceHandler.RaceState != RaceHandler.RESET)
      {
         bSendRaceData = true;
         bUpdateLights = true;
         return false;
      }
      else
      {
         RaceHandler.SetNumberOfDogs(1);
         return true;
      }
   }
   else if (ActionType == "SetRerunsOff")
   {
      if (!ActionObj.containsKey("actionData") || (RaceHandler.RaceState != RaceHandler.RESET))
      {
         // ReturnError = "No actionData found!";
         return false;
      }
      bool _bRerunsOff = ActionObj["actionData"]["rerunsOff"];
      // log_d("Received rerunsOff: %o", _bRerunsOff);
      if (_bRerunsOff)
         RaceHandler.ToggleRerunsOffOn(1);
      else
         RaceHandler.ToggleRerunsOffOn(0);
      return true;
   }
   else
   {
      // ReturnError = "Unknown action received!";
      log_d("Unknown action received: %s", ActionType.c_str());
      return false;
   }
}

void WebHandlerClass::_SendLightsData(int8_t iClientId)
{
   if (_iNumOfConsumers == 0)
      return;

   bUpdateLights = false;
   stLightsState LightStates = LightsController.GetLightsState();
   // log_d("Getting Lights state");
   StaticJsonDocument<96> jsonLightsDoc;
   JsonObject JsonRoot = jsonLightsDoc.to<JsonObject>();

   JsonArray JsonLightsData = JsonRoot.createNestedArray("LightsData");
   copyArray(LightStates.State, JsonLightsData);

   size_t len = measureJson(jsonLightsDoc);
   std::shared_ptr<std::vector<uint8_t>> buffer;
   buffer = std::make_shared<std::vector<uint8_t>>(len);

   if (std::move(buffer))
   {
      serializeJson(jsonLightsDoc, (char *)buffer->data(),len);
      //std::string strBufferToPrint = (char *)buffer->data();
      //log_d("LightsData buffer to send: %s", (strBufferToPrint.erase (strBufferToPrint.find ("xV"), strBufferToPrint [strBufferToPrint.length() - 2])).c_str());
      if (iClientId == -1)
      {
         _ws->textAll(std::move(buffer));
         /*for (uint8_t i = 1; i <= _ws->count(); i++)
         {
            if (_bIsConsumerArray[i])
            {
               AsyncWebSocketClient *client = _ws->client(i);
               client->text(buffer);
            }
         }*/
      }
      else
      {
         // log_d("Specific update. Sending to client %i", iClientId);
         AsyncWebSocketClient *client = _ws->client(iClientId);
         client->text(std::move(buffer));
      }
      _lLastBroadcast = millis();
   }
}

void WebHandlerClass::_SendRaceData(int iRaceId, int8_t iClientId)
{
   if (_iNumOfConsumers == 0)
      return;

   StaticJsonDocument<bsRaceData> JsonRaceDataDoc;
   JsonObject JsonRoot = JsonRaceDataDoc.to<JsonObject>();
   JsonObject JsonRaceData = JsonRoot.createNestedObject("RaceData");

   if (bUpdateRaceData)
   {
      JsonRaceData["id"] = RaceHandler.iCurrentRaceId + 1;
      JsonRaceData["racingDogs"] = RaceHandler.iNumberOfRacingDogs;
   }
   if (bUpdateThisRaceDataField[elapsedTime] || bUpdateTimerWebUIdata || bUpdateRaceData)
   {
      JsonRaceData["elapsedTime"] = RaceHandler.GetRaceTime();
      bUpdateThisRaceDataField[elapsedTime] = false;
   }
   if (bUpdateThisRaceDataField[cleanTime] || bUpdateTimerWebUIdata || bUpdateRaceData)
   {
      JsonRaceData["cleanTime"] = RaceHandler.GetCleanTime();
      bUpdateThisRaceDataField[cleanTime] = false;
   }
   if (bUpdateThisRaceDataField[raceState] || bUpdateRaceData)
   {
      JsonRaceData["raceState"] = RaceHandler.RaceState;
      bUpdateThisRaceDataField[raceState] = false;
   }
   if (bUpdateThisRaceDataField[rerunsOff] || bUpdateRaceData)
   {
      JsonRaceData["rerunsOff"] = RaceHandler.bRerunsOff;
      bUpdateThisRaceDataField[rerunsOff] = false;
   }

   JsonArray JsonDogDataArray = JsonRaceData.createNestedArray("dogData");
   // Update dogs times, crossing/entry times and re-run info
   for (int i = 0; i < RaceHandler.iNumberOfRacingDogs; i++)
   {
      JsonObject JsonDogData = JsonDogDataArray.createNestedObject();
      JsonDogData["dogNr"] = i;
      if (bUpdateThisRaceDataField[i + 4] || bUpdateTimerWebUIdata || bUpdateRaceData)
      {
         JsonDogData["fault"] = (RaceHandler._bDogFaults[i] || RaceHandler._bDogManualFaults[i]);
         bUpdateThisRaceDataField[i + 4] = false;
      }
      if (bUpdateThisRaceDataField[i + 8] || bUpdateTimerWebUIdata || bUpdateRaceData)
      {
         if (i == RaceHandler.iCurrentDog)
            JsonDogData["running"] = true;
         else
            JsonDogData["running"] = false;
         bUpdateThisRaceDataField[i + 8] = false;
      }
      if (bUpdateThisRaceDataField[i] || bUpdateTimerWebUIdata || bUpdateRaceData)
      {
         JsonArray JsonDogDataTimingArray = JsonDogData.createNestedArray("timing");
         char cForJson[9];
         for (uint8_t i2 = 0; i2 <= RaceHandler.iDogRunCounters[i]; i2++)
         {
            JsonObject DogTiming = JsonDogDataTimingArray.createNestedObject();
            RaceHandler.GetDogTime(i, i2).toCharArray(cForJson, 9);
            DogTiming["time"] = cForJson;
            RaceHandler.GetCrossingTime(i, i2).toCharArray(cForJson, 9);
            DogTiming["crossing"] = cForJson;
         }
         bUpdateThisRaceDataField[i] = false;
      }
   }
   
   //JsonRaceData.add(JsonRedNodeRaceData);
   log_d("Collected own racedata, length: %i\r\n", measureJson(JsonRaceData));

   /*if (_bBlueNodePresent)
   {
      log_d("Requesting blue ETS racedata...\r\n");
   #define USE_STRING

   #ifdef USE_STRING
      String strJsonBlueNodeRaceData = BlueNodeHandler.getBlueNodeRaceData();
      log_d("Blue ETS racedata: %s\r\n", strJsonBlueNodeRaceData.c_str());
      DeserializationError error = deserializeJson(JsonBlueNodeRaceDataDoc, strJsonBlueNodeRaceData);
      //char * strJsonBlueNodeRaceData = BlueNodeHandler.getBlueNodeRaceData2();
      //log_d("Blue node racedata: %s, length: %i\r\n", strJsonBlueNodeRaceData, strlen(strJsonBlueNodeRaceData));
      //DeserializationError error = deserializeJson(jsonBlueNodeRaceDataDoc, strJsonBlueNodeRaceData, strlen(strJsonBlueNodeRaceData));
      if (error)
         log_e("Error parsing json data from Blue ETS: %s\r\n", error.c_str());
      JsonBlueNodeRaceData = JsonBlueNodeRaceDataDoc.as<JsonObject>();
   #else
      JsonObject JsonBlueNodeRaceData = BlueNodeHandler.getBlueNodeRaceData1();

   #endif // USE_STRING

      String strJsonRaceDataTest;
      serializeJson(JsonBlueNodeRaceData, strJsonRaceDataTest);
      log_d("Got json Blue ETS racedata: %s, length: %i\r\n", strJsonRaceDataTest.c_str(), measureJson(JsonBlueNodeRaceData));

      if (measureJson(JsonBlueNodeRaceData) > 2)
         JsonRaceData.add(JsonBlueNodeRaceData);
      else
         log_i("Got invalid Blue ETS racedata (length: %i)", measureJson(JsonBlueNodeRaceData));
   }*/

   bUpdateTimerWebUIdata = false;
   bUpdateRaceData = false;

   size_t len = measureJson(JsonRaceDataDoc);
   std::shared_ptr<std::vector<uint8_t>> buffer;
   buffer = std::make_shared<std::vector<uint8_t>>(len);
   
   if (std::move(buffer))
   {
      serializeJson(JsonRaceDataDoc, (char *)buffer->data(),len);
      std::string strBufferToPrint = (char *)buffer->data();
      log_d("RaceData buffer to send:\n %s", (strBufferToPrint.erase (strBufferToPrint.find ("xV"), strBufferToPrint [strBufferToPrint.length() - 2])).c_str());
      if (iClientId == -1)
      {
         _ws->textAll(std::move(buffer));
      }
      else
      {
         // log_d("Specific update. Sending to client %i", iClientId);
         AsyncWebSocketClient *client = _ws->client(iClientId);
         client->text(std::move(buffer));
      }
      _lLastRaceDataBroadcast = _lLastBroadcast = millis();
      bSendRaceData = false;
   }
}

bool WebHandlerClass::_ProcessConfig(JsonArray newConfig, String *ReturnError)
{
   bool save = false;
   log_d("Config has %i elements", newConfig.size());
   for (unsigned int i = 0; i < newConfig.size(); i++)
   {
      String key = newConfig[i]["name"];
      String value = newConfig[i]["value"];

      if (value != SettingsManager.getSetting(key))
      {
         log_d("Storing %s = %s", key.c_str(), value.c_str());
         SettingsManager.setSetting(key, value);
         save = true;
         if (key == "RunDirectionInverted")
            RaceHandler.ToggleRunDirection();
         else if (key == "StartingSequenceNAFA")
            LightsController.ToggleStartingSequence();
         else if (key == "Accuracy3digits")
            RaceHandler.ToggleAccuracy();
         else if (key == "CommaInCsv")
            SDcardController.ToggleDecimalSeparator();
      }
   }

   if (save)
   {
      SettingsManager.saveSettings();
      bSendRaceData = true;
      // Schedule system reboot to activate new settings in 5s
      // SystemManager.scheduleReboot(millis() + 5000);
   }

   return true;
}

bool WebHandlerClass::_GetData(String dataType, JsonObject Data)
{
   if (dataType == "config")
   {
      Data["APName"] = SettingsManager.getSetting("APName");
      Data["APPass"] = SettingsManager.getSetting("APPass");
      Data["AdminPass"] = SettingsManager.getSetting("AdminPass");
      Data["RunDirectionInverted"] = SettingsManager.getSetting("RunDirectionInverted");
      Data["StartingSequenceNAFA"] = SettingsManager.getSetting("StartingSequenceNAFA");
      Data["LaserOnTimer"] = SettingsManager.getSetting("LaserOnTimer");
      Data["Accuracy3digits"] = SettingsManager.getSetting("Accuracy3digits");
      Data["CommaInCsv"] = SettingsManager.getSetting("CommaInCsv");
      // log_d("AdminPass: %s, RunDirection: %s", SettingsManager.getSetting("AdminPass").c_str(), SettingsManager.getSetting("RunDirectionInverted"));
   }
   else if (dataType == "triggerQueue")
   {
      JsonArray triggerQueue = Data.createNestedArray("triggerQueue");

      for (auto &trigger : RaceHandler._OutputTriggerQueue)
      {
         JsonObject triggerObj = triggerQueue.createNestedObject();
         triggerObj["sensorNum"] = trigger.iSensorNumber;
         triggerObj["triggerTime"] = trigger.llTriggerTime - RaceHandler.llRaceStartTime;
         triggerObj["state"] = trigger.iSensorState;
      }
   }
   else
   {
      Data["error"] = "Unknown datatype (" + dataType + ") requested!";
      return false;
   }

   return true;
}

void WebHandlerClass::_SendSystemData(int8_t iClientId)
{
   if (_iNumOfConsumers == 0)
      return;
   
   if (!RaceHandler.bRunDirectionInverted)
      _strRunDirection = (char *)"->";
   else
      _strRunDirection = (char *)"<-";

   StaticJsonDocument<192> JsonSystemDataDoc;
   JsonObject JsonRoot = JsonSystemDataDoc.to<JsonObject>();

   JsonObject JsonSystemData = JsonRoot.createNestedObject("SystemData");
   JsonSystemData["ut"] = MICROS / 1000000;
   JsonSystemData["FW"] = (char *)FW_VER;
   JsonSystemData["Tag"] = _iPwrOnTag;
   JsonSystemData["RID"] = RaceHandler.iCurrentRaceId + 1;
   JsonSystemData["clients"] = _ws->count();
   JsonSystemData["sTime"] = (char *)GPSHandler.GetUtcDateAndTime();
   JsonSystemData["bat"] = BatterySensor.GetBatteryPercentage();
   JsonSystemData["dir"] = _strRunDirection;

   size_t len = measureJson(JsonSystemDataDoc);
   std::shared_ptr<std::vector<uint8_t>> buffer;
   buffer = std::make_shared<std::vector<uint8_t>>(len);

   if (std::move(buffer))
   {
      serializeJson(JsonSystemDataDoc, (char *)buffer->data(),len);
      //std::string strBufferToPrint = (char *)buffer->data();
      //log_d("SystemData buffer to send: %s.",(strBufferToPrint.erase (strBufferToPrint.find ("xV"), strBufferToPrint [strBufferToPrint.length() - 2])).c_str());
      if (iClientId == -1)
      {
         _ws->textAll(std::move(buffer));
      }
      else
      {
         // log_d("Specific update. Sending to client %i", iClientId);
         AsyncWebSocketClient *client = _ws->client(iClientId);
         client->text(std::move(buffer));
      }
      _lLastSystemDataBroadcast = _lLastBroadcast = millis();
   }
}

void WebHandlerClass::disconnectWsClient(IPAddress ipDisconnectedIP)
{
   uint8_t iId = 1;
   for (auto &isConsumer : _bIsConsumerArray)
   {
      if (isConsumer)
      {
         log_d("Getting client obj for id %i", iId);
         AsyncWebSocketClient *client = _ws->client(iId);
         IPAddress ip = client->remoteIP();
         log_d("IP to check: %s. IP of the client: %s", ipDisconnectedIP.toString().c_str(), ip.toString().c_str());
         if (client && client->status() == WS_CONNECTED && ip == ipDisconnectedIP)
         {
            log_d("Closing client %i", iId);
            client->close();
            _iNumOfConsumers--;
            _bIsConsumerArray[iId] = false;
         }
      }
      iId++;
   }
}

void WebHandlerClass::_onAuth(AsyncWebServerRequest *request)
{
   if (!_authenticate(request))
      return request->requestAuthentication("", false);
   IPAddress ip = request->client()->remoteIP();
   unsigned long now = millis();
   unsigned short index;
   for (index = 0; index < WS_TICKET_BUFFER_SIZE; index++)
   {
      if (_ticket[index].ip == ip)
         break;
      if (_ticket[index].timestamp == 0)
         break;
      if (now - _ticket[index].timestamp > WS_TIMEOUT)
         break;
   }
   if (index == WS_TICKET_BUFFER_SIZE)
   {
      request->send(429);
   }
   else
   {
      _ticket[index].ip = ip;
      _ticket[index].timestamp = now;
      request->send(204);
   }
}

bool WebHandlerClass::_authenticate(AsyncWebServerRequest *request)
{
   String password = SettingsManager.getSetting("AdminPass");
   char httpPassword[password.length() + 1];
   password.toCharArray(httpPassword, password.length() + 1);
   bool bAuthResult = request->authenticate("Admin", httpPassword);
   if (!bAuthResult)
   {
      log_e("Admin user failed to login!");
   }
   return bAuthResult;
}

bool WebHandlerClass::_wsAuth(AsyncWebSocketClient *client)
{
   IPAddress ip = client->remoteIP();
   unsigned short index = 0;

   // TODO: Here be dragons, this way of 'authenticating' is all but secure
   // We are just storing the client's IP and an expiration timestamp of now + 30 mins
   // So if someone logs in, then disconnects from the WiFi, and then someone else connects
   // this new user will/could have the same IP as the previous user, and will be authenticated :(

   for (index = 0; index < WS_TICKET_BUFFER_SIZE; index++)
   {
      log_d("Checking ticket: %i, ip: %s, time: %ul", index, _ticket[index].ip.toString().c_str(), _ticket[index].timestamp);
      if ((_ticket[index].ip == ip) && (millis() - _ticket[index].timestamp < WS_TIMEOUT))
         break;
   }

   if (index == WS_TICKET_BUFFER_SIZE)
   {
      log_e("[WEBSOCKET] Validation check failed");
      client->text("{\"success\": false, \"error\": \"You shall not pass!!!! Please authenticate first :-)\", \"authenticated\": false}");
      _lLastBroadcast = millis();
      return false;
   }

   return true;
}

void WebHandlerClass::_onHome(AsyncWebServerRequest *request)
{

   // Check if the client already has the same version and respond with a 304 (Not modified)
   if (request->header("If-Modified-Since").equals(_last_modified))
   {
      request->send(304);
   }
   else
   {
#ifndef WebUIonSDcard
      // Dump the byte array in PROGMEM with a 200 HTTP code (OK)
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
      // Tell the browswer the contemnt is Gzipped
      response->addHeader("Content-Encoding", "gzip");
#else
      AsyncWebServerResponse *response = request->beginResponse(SD_MMC, "/index.htm", "text/html");
#endif
      // And set the last-modified datetime so we can check if we need to send it again next time or not
      response->addHeader("Last-Modified", _last_modified);
      request->send(response);
   }
}

void WebHandlerClass::_onFavicon(AsyncWebServerRequest *request)
{
   if (request->header("If-Modified-Since").equals(_last_modified))
   {
      request->send(304);
   }
   else
   {
#ifndef WebUIonSDcard
      AsyncWebServerResponse *response = request->beginResponse_P(200, "image/png", index_html_gz, index_html_gz_len);
      response->addHeader("Content-Encoding", "gzip");
#else
      AsyncWebServerResponse *response = request->beginResponse(SD_MMC, "/favicon.ico", "image/png");
#endif
      // And set the last-modified datetime so we can check if we need to send it again next time or not
      response->addHeader("Last-Modified", _last_modified);
      request->send(response);
   }
}

bool WebHandlerClass::RedNodeConnected()
{
   return _bBlueNodePresent;
}

void WebHandlerClass::_CheckRedNodeStatus()
{
   if (!_RedNodeStatus.Configured)
      return;

   if (millis() - _RedNodeStatus.LastCheck > 1200)
   {
      log_d("Checking Blue ETS, if any...\r\n");
      _RedNodeStatus.LastCheck = millis();
      log_d("Blue ETS wsClient->Status: %i\r\n", _RedNodeStatus.client->status());
      if (_RedNodeStatus.client->status() != WS_CONNECTED)
      {
         log_d("ws client status says Red ETS is disconnected (actual status: %i)\r\n", _RedNodeStatus.client->status());
         _DisconnectRedNode();
      }
      else if (millis() - _RedNodeStatus.LastReply > 3600)
      {
         log_d("Disconnecting Red ETS due to timeout (last pong: %lu, now: %lu)\r\n", _RedNodeStatus.LastReply, millis());
         _DisconnectRedNode();
      }
      else
      {
         log_d("I am pinging the Red ETS at %lu\r\n", millis());
         _RedNodeStatus.client->ping();
      }
   }
}

void WebHandlerClass::_DisconnectRedNode()
{
   _RedNodeStatus.Configured = false;
   _ws->close(_RedNodeStatus.ClientID);
   log_i("Red ETS disconnected!");
}

WebHandlerClass WebHandler;
