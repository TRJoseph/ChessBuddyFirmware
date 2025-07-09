#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "boardcontrol.h"
#include <WiFiClientSecure.h>
#include <esp_task_wdt.h>


// Render endpoint
const char* serverURL = "https://chess-engine-service.onrender.com/get_move";

String buildMovesString() {
  String moves = "";
  for (int i = 0; i < moveCount; i++) {
    moves += moveHistory[i];
    if (i < moveCount - 1) {
      moves += " ";
    }
  }
  return moves;
}


void getBestMoveFromServer() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure();  // cert verification disabled for development

        String fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        String moves = buildMovesString();

        // builds JSON payload
        DynamicJsonDocument doc(512);
        doc["fen"] = fen;
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
                handleArmMove(move);
            } else {
                Serial.println("Failed to parse JSON");
            }
        } else {
            Serial.print("HTTP request failed: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("WiFi not connected");
    }
}

void getBestMoveTask(void *parameter) {
  esp_task_wdt_init(60, true);
  getBestMoveFromServer();
  vTaskDelete(NULL);
}