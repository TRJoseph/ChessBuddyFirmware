#include <Preferences.h>
#include "FT6336U.h"
#include <Misc_Definitions.h>
#include "User_Setup.h"
#include <WiFi.h>
#include "wlan.h"
#include "gui.h"


Preferences prefs;

WifiScanState currentWifiState = WIFI_IDLE;
struct Network* networksList = NULL;
int networkCount = 0;
unsigned long scanStartTime = 0;

int max_attempts = 10;
int attempt = 1;


void setup_preferences() {

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // open prefrences in RW mode
  prefs.begin("WLANPrefs", false);

  // check for ssid key
  bool IsSsidKey = prefs.isKey("ssid");
  bool IsPasswordKey = prefs.isKey("networkPass");

  if(IsSsidKey == false || IsPasswordKey == false) {
    // dont connect, preferences are missing a key
    Serial.write("Wireless credential keys not found.\n");
  } else {
    // both keys are available, attempt to connect to network
    Serial.write("Wireless credential keys found, attempting to connect to network...\n");
    String ssid = prefs.getString("ssid");
    String pswrd = prefs.getString("networkPass");
    

    // TODO: maybe do null check on preferences?
    connectToWifiNetwork(ssid, pswrd);
  }
  prefs.end();

}


void check_wifi_status_timer_cb(lv_timer_t * timer) {


    // check if wifi is connected
    processWifiState();

    // if wifi is connected stop the timer
    if(WiFi.status() == WL_CONNECTED || 
        currentWifiState == WIFI_CONNECTION_FAILED) {
        lv_timer_del(timer);
    }
}


void connectToWifiNetwork(const String& ssid, const String& password) {
  if (currentWifiState == WIFI_IDLE || 
      currentWifiState == WIFI_CONNECTION_FAILED) {
    
    currentWifiState = WIFI_CONNECTING;
    scanStartTime = millis(); // Reuse for connection timeout
    attempt = 1;
    
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.printf("Connecting to %s...\n", ssid.c_str());
    
    // Update UI to show connecting state
    //showWifiConnecting(ssid);

    lv_timer_create(check_wifi_status_timer_cb, 500, NULL);
  }
}


void connectToWifiNetworkBlocking(const String& ssid, const String& password) {
    Serial.println("Connecting to WiFi...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    WiFi.begin(ssid.c_str(), password.c_str());
    attempt = 1;
    currentWifiState = WIFI_CONNECTING;


    while (WiFi.status() != WL_CONNECTED && attempt < 10) {
        delay(500);
        Serial.print(".");
        wl_status_t status = WiFi.status();
        
        if (status == WL_CONNECT_FAILED) {
            Serial.println("Connection failed (wrong password or AP unreachable).");
            currentWifiState = WIFI_CONNECTION_FAILED;
            break;
        }
        
        if (status == WL_NO_SSID_AVAIL) {
            Serial.println("SSID not found.");
            currentWifiState = WIFI_CONNECTION_FAILED;
            break;
        }

        attempt++;
    }

    WiFi.mode(WIFI_STA);

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        currentWifiState = WIFI_CONNECTED;

        prefs.begin("WLANPrefs", false);
        // store wifi credentials in preferences so they persist after reboot
        prefs.putString("ssid", ssid.c_str());
        // TODO: LIKELY NEED TO ENCRYPT WIFI CREDENTIALS INSTEAD OF STORING IN PLAIN TEXT
        prefs.putString("networkPass", password.c_str());

        prefs.end();

    } else {
        currentWifiState = WIFI_CONNECTION_FAILED;
        Serial.println("Failed to connect after multiple attempts.");
    }

}


void disconnectFromWifiNetwork() {
  Serial.printf("Disconnecting from %s...\n", WiFi.SSID().c_str());
  WiFi.disconnect(true);
  setWifiState(WIFI_IDLE);
}


void setWifiState(WifiScanState state) {
    currentWifiState = state;
}

WifiScanState getWifiState() {
    return currentWifiState;
}

int getWifiSignalStrength() {
    return WiFi.RSSI();
}

void freeNetworks(struct Network* networks) {
    if (networks != NULL) {
        delete[] networks;
    }
}

void processWifiState() {

  switch (currentWifiState) {
    case WIFI_SCANNING: {

      int scanResult = WiFi.scanComplete();

      if(scanResult >= 0) {
        networkCount = scanResult;

        if (networkCount == 0) {
          // count should be 0
          updateWifiNetworkList(networkCount, networksList);
        } else {
          networksList = new struct Network[networkCount];
          for (int i = 0; i < networkCount; ++i) {
            networksList[i].num = i + 1;
            networksList[i].ssid = WiFi.SSID(i).c_str();
            networksList[i].rssi = WiFi.RSSI(i);
            networksList[i].channel = WiFi.channel(i);
            networksList[i].encryptionType = WiFi.encryptionType(i);
            switch (WiFi.encryptionType(i)) {
              case WIFI_AUTH_OPEN:
                networksList[i].encryptionType = "open";
                break;
              case WIFI_AUTH_WEP:
                networksList[i].encryptionType = "WEP";
                break;
              case WIFI_AUTH_WPA_PSK:
                networksList[i].encryptionType = "WPA";
                break;
              case WIFI_AUTH_WPA2_PSK:
                networksList[i].encryptionType = "WPA2";
                break;
              case WIFI_AUTH_WPA_WPA2_PSK:
                networksList[i].encryptionType = "WPA+WPA2";
                break;
              case WIFI_AUTH_WPA2_ENTERPRISE:
                networksList[i].encryptionType = "WPA2-EAP";
                break;
              case WIFI_AUTH_WPA3_PSK:
                networksList[i].encryptionType = "WPA3";
                break;
              case WIFI_AUTH_WPA2_WPA3_PSK:
                networksList[i].encryptionType = "WPA2+WPA3";
                break;
              case WIFI_AUTH_WAPI_PSK:
                networksList[i].encryptionType = "WAPI";
                break;
              default:
                networksList[i].encryptionType = "unknown";
            }
            delay(10);
          }
        }
        currentWifiState = WIFI_IDLE;

        // TODO: update UI for menu
        updateWifiNetworkList(networkCount, networksList);
        WiFi.scanDelete();
      } else if (scanResult == WIFI_SCAN_FAILED) {
        // Scan failed
        currentWifiState = WIFI_CONNECTION_FAILED;
        Serial.println("WiFi scan failed");
        
        updateWifiWidget(WIFI_CONNECTION_FAILED);
      }
      else if (millis() - scanStartTime > 10000) {
        // Timeout after 10 seconds
        WiFi.scanDelete();
        currentWifiState = WIFI_CONNECTION_FAILED;
        Serial.println("WiFi scan timeout");
        
        // Update UI to show timeout
        updateWifiWidget(WIFI_CONNECTION_FAILED);
      }
      break;
    }
    case WIFI_CONNECTING: {
        if (WiFi.status() == WL_CONNECTED) {
            currentWifiState = WIFI_CONNECTED;
            Serial.println("WiFi connected");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            
            // Update UI to show connected state
            updateWifiWidget(currentWifiState);
          } 
          else if (millis() - scanStartTime > 10000 || attempt == max_attempts) {
            // Connection timeout after 20 seconds
            currentWifiState = WIFI_CONNECTION_FAILED;
            Serial.println("WiFi connection failed or timed out");
            
            // Update UI to show connection failure
            updateWifiWidget(currentWifiState);
          }
      break;
    }
    case WIFI_CONNECTED: {
      // TODO
    }
  }
}

void check_scan_status_timer_cb(lv_timer_t * timer) {
    int result = WiFi.scanComplete();

    if (result == WIFI_SCAN_RUNNING) {
        return;
    }

    lv_timer_del(timer);

    processWifiState(); 

    updateWifiWidget(currentWifiState);
}

// const char* wifiScanStateToString(WifiScanState state) {
//   switch (state) {
//     case WIFI_IDLE: return "IDLE";
//     case WIFI_SCANNING: return "SCANNING";
//     case WIFI_SCAN_COMPLETE: return "SCAN COMPLETE";
//     case WIFI_CONNECTING: return "CONNECTING";
//     case WIFI_CONNECTED: return "CONNECTED";
//     case WIFI_CONNECTION_FAILED: return "CONNECTION FAILED";
//     default: return "UNKNOWN";
//   }
// }



void startWifiScan() {
   wl_status_t wifiStatus = WiFi.status();

  if (wifiStatus == WL_DISCONNECTED || currentWifiState == WIFI_CONNECTION_FAILED) {
    Serial.println("Resetting WiFi before scan");
    WiFi.disconnect(true);  // Disconnect with 'true' to clear saved settings
    delay(200);  // Give it time to reset
    WiFi.mode(WIFI_STA); 
  }

  // Serial.print("Current WiFi status: ");
  // switch (wifiStatus) {
  //   case WL_IDLE_STATUS:
  //     Serial.println("Idle");
  //     break;
  //   case WL_NO_SSID_AVAIL:
  //     Serial.println("No SSID Available");
  //     break;
  //   case WL_SCAN_COMPLETED:
  //     Serial.println("Scan Completed");
  //     break;
  //   case WL_CONNECTED:
  //     Serial.println("Connected");
  //     break;
  //   case WL_CONNECT_FAILED:
  //     Serial.println("Connection Failed");
  //     break;
  //   case WL_CONNECTION_LOST:
  //     Serial.println("Connection Lost");
  //     break;
  //   case WL_DISCONNECTED:
  //     Serial.println("Disconnected");
  //     break;
  //   default:
  //     Serial.println("Unknown");
  //     break;
  // }

  // Serial.print("Current WifiScanState: ");
  // Serial.println(wifiScanStateToString(currentWifiState));

  if (currentWifiState == WIFI_IDLE || currentWifiState == WIFI_CONNECTION_FAILED) {
  
    currentWifiState = WIFI_SCANNING;
    scanStartTime = millis();
    
    //Free previous results if they exist
    if (networksList != NULL) {
      freeNetworks(networksList);
      networksList = NULL;
    }
    
    // Start asynchronous WiFi scan
    WiFi.scanNetworks(true); // true = async mode
    
    Serial.println("WiFi scan started");
    
    lv_timer_create(check_scan_status_timer_cb, 500, NULL);
    // TODO: update UI to show wifi scanning animation in lvgl
    //updateWifiScanningUI(true);
  }

  // IF WIFI IS IN CONNECTED STATE AND A SCAN IS ATTEMPTED, SIMPLY REMOVE THE LOADING SPINNER AND ADD THE CONNECTED NETWORK ONLY
  if(currentWifiState == WIFI_CONNECTED) {

    struct Network* currentNetwork = new struct Network[1];

    currentNetwork[0].num = 1;
    currentNetwork[0].ssid = strdup(WiFi.SSID().c_str()); // Make a copy of the SSID string
    currentNetwork[0].rssi = WiFi.RSSI();
    currentNetwork[0].channel = WiFi.channel();
    currentNetwork[0].encryptionType = "";

    updateWifiNetworkList(1, currentNetwork);
  }
}

