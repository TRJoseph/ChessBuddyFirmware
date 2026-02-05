#ifndef SERVERINTERFACE
#define SERVERINTERFACE

#include "WiFi.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <esp_task_wdt.h>

class Main_Controller;

class ServerInterface {
private:
    // Render endpoint
    const char* serverURL = "https://chess-engine-service.onrender.com/get_move";

    // reference to the main controller object
    Main_Controller& main_controller;

    String buildMovesString();
public:
    ServerInterface(Main_Controller& mainControllerRef);

    /* FUNCTION DEFINITIONS */
    
    static void getBestMoveTask(void *parameter);
    void getBestMoveFromServer();

};

#endif