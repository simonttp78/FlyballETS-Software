// BlueNodeHandler.h

#ifndef _BlueNodeHandler_h
#define _BlueNodeHandler_h

#include <Arduino.h>
#include <global.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <SettingsManager.h>
#include <enums.h>
#define CONNECT_CHECK 1000

class BlueNodeHandlerClass
{
protected:
    IPAddress _RemoteIP;
    bool _bConnected;
    bool _bIAmBlueNode;
    bool _bBlueNodeConfigured;
    bool _bWSConnectionStarted;
    bool _bBlueNodeAnnounced;
    bool _bConsumerAnnounced;
    bool _bInitialized = false;
    unsigned long _ulLastConnectCheck;
    unsigned long _ulLastSystemDataReceived;
    WebSocketsClient _wsClient;

    typedef struct
    {
        unsigned long LastCheck;
        unsigned long LastReply;
    } stBlueNodeStatus;
    stBlueNodeStatus _BlueNodeStatus;

    void _ConnectRemote();
    void _WsEvent(WStype_t type, uint8_t *payload, size_t length);
    void _SetDisconnected();
    void _AnnounceBlueNodeIfApplicable();
    void _AnnounceConsumerIfApplicable();
    void _WsCloseConnection();
    bool _ConnectionNeeded();
    void _TestConnection();
    String _strJsonRaceData;
    StaticJsonDocument<bsActionScheduleStartRace> _jdocClientInput;
    JsonObject _jsonClientInput;
    StaticJsonDocument<bsRaceData> _jdocRaceData;
    JsonObject _jsonRaceData;

public:
    void init();
    void loop();
    void configureBlueNode(IPAddress ipBlueNodeIP);
    void removeBlueNode();
    bool blueNodePresent();
    void resetConnection();
    bool sendToBlueNode(String strMessage);
    bool GetConnectionStatus();
    String getBlueNodeRaceData();
    JsonObject getBlueNodeRaceData1();
    char *getBlueNodeRaceData2();
};

extern BlueNodeHandlerClass BlueNodeHandler;

#endif