#include <BlueNodeHandler.h>

void BlueNodeHandlerClass::init()
{
    if (_bInitialized)
        return;
    this->_bConnected = false;
    this->_bIAmBlueNode = false;
    this->_bWSConnectionStarted = false;
    this->_bBlueNodeAnnounced = false;
    this->_bConsumerAnnounced = false;
    this->_bBlueNodeConfigured = false;
    if (SettingsManager.getSetting("OperationMode").toInt() == SystemModes::BLUE)
    {
        this->_bIAmBlueNode = true;
        this->_RemoteIP = IPAddress(192, 168, 20, 1);
    }

    this->_wsClient.onEvent(std::bind(&BlueNodeHandlerClass::_WsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // this->_wsClient.setReconnectInterval(CONNECT_CHECK);
    log_i("BlueNodeHandler initialized!");

    const size_t bufferSize = 384;
    StaticJsonDocument<bufferSize> _jdocRaceData;
    JsonObject _jsonRaceData = _jdocRaceData.to<JsonObject>();
    //_jsonRaceData = _jdocRaceData.to<JsonObject>();
    _bInitialized = true;
}
void BlueNodeHandlerClass::loop()
{
    if (millis() - this->_ulLastConnectCheck > CONNECT_CHECK)
    {
        this->_ulLastConnectCheck = millis();
        log_v("C: %i, CN: %i, CS: %i, S: %i, SC: %i, strRDLength: %i\r\n", _bConnected, _ConnectionNeeded(), _bWSConnectionStarted, _bIAmBlueNode, _bBlueNodeConfigured, _strJsonRaceData.length());
        if (!_bConnected && this->_ConnectionNeeded() && !_bWSConnectionStarted)
        {
            this->_ConnectRemote();
        }

        if (this->_bConnected && !this->_ConnectionNeeded())
        {
            this->_WsCloseConnection();
        }
    }

    if (this->_bConnected || this->_ConnectionNeeded())
    {
        this->_wsClient.loop();
    }

    _TestConnection();
}

void BlueNodeHandlerClass::configureBlueNode(IPAddress ipBlueNodeIP)
{
    this->_RemoteIP = ipBlueNodeIP;
    this->_bBlueNodeConfigured = true;
    this->_bWSConnectionStarted = false;
    this->_SetDisconnected();
}

void BlueNodeHandlerClass::removeBlueNode()
{
    // this->_RemoteIP = IPAddress(0, 0, 0, 0);
    this->_bBlueNodeConfigured = false;
    this->_bWSConnectionStarted = false;
}

bool BlueNodeHandlerClass::blueNodePresent()
{
    return (_bConnected && !_bIAmBlueNode);
}

void BlueNodeHandlerClass::_ConnectRemote()
{
    this->_wsClient.begin(this->_RemoteIP.toString(), 80, "/ws");
    this->_bWSConnectionStarted = true;
    log_d("Initiated connection to %s\r\n", this->_RemoteIP.toString().c_str());
}

void BlueNodeHandlerClass::_WsEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        this->_SetDisconnected();
        if (!_bIAmBlueNode)
        {
            this->_bConsumerAnnounced = false;
        }
        break;

    case WStype_CONNECTED:
        this->_bConnected = true;
        this->_bWSConnectionStarted = false;
        this->_AnnounceBlueNodeIfApplicable();
        this->_AnnounceConsumerIfApplicable();
        log_d("Connected to url: %s\n", payload);
        _BlueNodeStatus.LastReply = millis();

        // send message to server when Connected
        // this->_wsClient.sendTXT("Connected");
        break;

    case WStype_TEXT:
    {
        log_d("At %lu I got text: %s\n", millis(), payload);
        _BlueNodeStatus.LastReply = millis();
        StaticJsonDocument<768> _jdocClientInput;
        DeserializationError error = deserializeJson(_jdocClientInput, payload);
        JsonObject _jsonClientInput = _jdocClientInput.as<JsonObject>();
        if (error)
        {
            log_e("Error parsing JSON: %s", error.c_str());
            // client->text("{\"error\":\"Invalid JSON received\"}");
            return;
        }

        /*DeserializationError error = deserializeJson(_jdocClientInput, (const char *)payload);
        if (error)
        {
            log_i("Error parsing JSON: %s!", error.c_str());
            return;
        }
        _jsonClientInput = _jdocClientInput.as<JsonObject>();*/

        if (_jsonClientInput.containsKey("RaceData"))
        {
            //_jsonRaceData = _jdocRaceData.to<JsonObject>();
            //_jsonRaceData = _jsonClientInput["RaceData"][0];
            JsonObject RaceDataResult = _jsonRaceData.createNestedObject("RaceData");
            JsonArray _jsonRaceData = _jdocRaceData["RaceData"].as<JsonArray>();

            _strJsonRaceData = "";
            serializeJson(_jsonRaceData, _strJsonRaceData);
            log_d("got racedata from Blue ETS: %s\r\n", _strJsonRaceData.c_str());
        }
        else if (_jsonClientInput.containsKey("SystemData"))
        {
            log_d("Got SystemData!\r\n");
            _ulLastSystemDataReceived = millis();
        }
        break;
    }

    case WStype_BIN:
        log_d("get binary length: %u\n", length);
        break;

    default:
        log_d("Received event %i", type);
    }
}

void BlueNodeHandlerClass::_SetDisconnected()
{
    if (!this->_bConnected)
        return;
    this->_bConnected = false;
    log_d("Disconnected!\n");
}

void BlueNodeHandlerClass::_AnnounceBlueNodeIfApplicable()
{
    if (!_bIAmBlueNode || _bBlueNodeAnnounced)
    {
        return;
    }
    String strJson = String("{\"action\": {\"actionType\": \"AnnounceBlue\"}}");
    this->_wsClient.sendTXT(strJson);
    this->_bBlueNodeAnnounced = true;

    // If we are Blue ETS and we have announced ourselves as such, we can close the connection
    log_d("Announed Blue ETS, disconnecting...\r\n");
    this->_WsCloseConnection();
}

void BlueNodeHandlerClass::_AnnounceConsumerIfApplicable()
{
    if (_bIAmBlueNode || _bConsumerAnnounced)
    {
        return;
    }
    String strJson = String("{\"action\": {\"actionType\": \"AnnounceConsumer\"}}");
    this->_wsClient.sendTXT(strJson);
    this->_bConsumerAnnounced = true;

    log_d("Announed Consumer!\r\n");
}

void BlueNodeHandlerClass::_WsCloseConnection()
{
    this->_wsClient.disconnect();
    this->_bConnected = false;
}

bool BlueNodeHandlerClass::_ConnectionNeeded()
{
    if (_bIAmBlueNode)
    {
        if (WiFi.status() != WL_CONNECTED || (_bIAmBlueNode && WiFi.localIP().toString().equals("0.0.0.0")))
        {
            return false;
        }
        return (!_bBlueNodeAnnounced);
    }
    else
    {
        return (_bBlueNodeConfigured || !_bConsumerAnnounced);
    }
}

void BlueNodeHandlerClass::resetConnection()
{
    this->_bBlueNodeAnnounced = false;
    this->_bConsumerAnnounced = false;
}

bool BlueNodeHandlerClass::sendToBlueNode(String strMessage)
{
    if (_bIAmBlueNode)
    {
        return false;
    }
    bool bResult = true;

    if (_bConnected)
    {
        _wsClient.sendTXT(strMessage);
    }
    else
    {
        bResult = false;
    }

    return bResult;
}

String BlueNodeHandlerClass::getBlueNodeRaceData()
{
    log_d("Blue ETS is returning racedata: %s\r\n", _strJsonRaceData.c_str());
    return _strJsonRaceData;
}

JsonObject BlueNodeHandlerClass::getBlueNodeRaceData1()
{
    String strJsonRaceData;
    serializeJson(_jdocRaceData, strJsonRaceData);
    log_d("Returning Blue ETS racedata: %s\r\n\r\n", strJsonRaceData.c_str());
    return _jsonRaceData;
}

/*char *BlueNodeHandlerClass::getBlueNodeRaceData2()
{

    char strJsonRaceData[measureJson(_jsonRaceData)];
    serializeJson(_jsonRaceData, strJsonRaceData, measureJson(_jsonRaceData));
    log_d("Returning Blue ETS racedata: %s\r\n\r\n", strJsonRaceData);
    return strJsonRaceData;
}*/

void BlueNodeHandlerClass::_TestConnection()
{
    // TODO: This connection test is not working...
    if (this->_ConnectionNeeded() && _bConnected)
    {
        if (millis() - _BlueNodeStatus.LastCheck > 1200)
        {
            _BlueNodeStatus.LastCheck = millis();
            log_d("Testing connection at %lu...\r\n", millis());
            log_d("Test result: %i\r\n", _wsClient.sendPing());
        }
        if (millis() - _BlueNodeStatus.LastReply > 6000)
        {
            log_d("Connection broken at %lu, reconnecting...\r\n", millis());
            _wsClient.disconnect();
            this->_SetDisconnected();
        }
    }
}

bool BlueNodeHandlerClass::GetConnectionStatus()
{
    return _bConnected;
}

BlueNodeHandlerClass BlueNodeHandler;