#include "serverinterface.h"
#include "main_control/maincontrol.h"

ServerInterface::ServerInterface(Main_Controller& mainControllerRef) : main_controller(mainControllerRef) {}

String ServerInterface::buildMovesString() {
  String moves = "";
  for (int i = 0; i < main_controller.moveCount; i++) {
    moves += main_controller.moveHistory[i];
    if (i < main_controller.moveCount - 1) {
      moves += " ";
    }
  }
  return moves;
}


void ServerInterface::getBestMoveFromServer() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure();  // cert verification disabled for development

        
        String moves = buildMovesString();

        // builds JSON payload
        DynamicJsonDocument doc(512);
        doc["fen"] = startPosFEN;
        if (moves.length() > 0) {
            doc["moves"] = moves;
        }

        String postData;
        serializeJson(doc, postData);

        http.begin(client, serverURL);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(postData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server response:");
            Serial.println(response);

            // get response
            DynamicJsonDocument respDoc(256);
            DeserializationError error = deserializeJson(respDoc, response);
            if (!error) {
                const char* move = respDoc["move"];
                Serial.print("Engine move: ");
                Serial.println(move);
                http.end();
                main_controller.handleArmMove(move);
            } else {
                Serial.println("Failed to parse JSON");
                http.end();
            }
        } else {
            Serial.print("HTTP request failed: ");
            Serial.println(httpResponseCode);
        }
    } else {
        Serial.println("WiFi not connected");
    }
}

void ServerInterface::getBestMoveTask(void *pvParameter) {
    ServerInterface* server_interface = static_cast<ServerInterface*>(pvParameter);
    if(!server_interface) return;
    //esp_task_wdt_init(60, true);
    server_interface->getBestMoveFromServer();
    vTaskDelete(NULL);
}