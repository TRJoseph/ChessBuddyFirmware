#include <Preferences.h>
#include "FT6336U.h"
#include <Main_Definitions.h>
#include "User_Setup.h"
#include <WiFi.h>
#include "wlan.h"
#include "gui.h"


Preferences prefs;

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


//Optional helper to convert encryption types to human-readable string
// String getEncryptionType(wifi_auth_mode_t type) {
//   switch (type) {
//     case WIFI_AUTH_OPEN: return "Open";
//     case WIFI_AUTH_WEP: return "WEP";
//     case WIFI_AUTH_WPA_PSK: return "WPA-PSK";
//     case WIFI_AUTH_WPA2_PSK: return "WPA2-PSK";
//     case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2-PSK";
//     case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Enterprise";
//     case WIFI_AUTH_WPA3_PSK: return "WPA3-PSK";
//     case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3-PSK";
//     default: return "Unknown";
//   }
// }

// void setup_preferences() {
//   Serial.begin(115200);
//   delay(1000); // Wait for Serial to initialize

//   WiFi.mode(WIFI_STA);  // Set WiFi to Station mode
//   WiFi.disconnect();    // Disconnect from any previous connections
//   delay(100);

//   Serial.println("\n🔍 Scanning for WiFi networks...");

//   int networksFound = WiFi.scanNetworks();

//   if (networksFound == 0) {
//     Serial.println("❌ No networks found.");
//   } else {
//     Serial.printf("✅ %d network(s) found:\n", networksFound);
//     for (int i = 0; i < networksFound; ++i) {
//       // Print SSID, RSSI (signal strength), and encryption type
//       Serial.printf("%d: %s (%ddBm) Encryption: %s\n", i + 1,
//                     WiFi.SSID(i).c_str(),
//                     WiFi.RSSI(i),
//                     getEncryptionType(WiFi.encryptionType(i)).c_str());
//       delay(10);
//     }
//   }
//   Serial.println("📡 Scan complete.\n");
// }

void debugCurrentWifiStatus() {


  wl_status_t wifiStatus = WiFi.status();
  Serial.print("Current WiFi status (int): ");
  Serial.println((int)wifiStatus);  // Print numeric value

  Serial.print("Current WiFi status (label): ");
  switch (wifiStatus) {
    case WL_IDLE_STATUS:
      Serial.println("Idle");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("No SSID Available");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("Scan Completed");
      break;
    case WL_CONNECTED:
      Serial.println("Connected");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("Connection Failed");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("Connection Lost");
      break;
    case WL_DISCONNECTED:
      Serial.println("Disconnected");
      break;
    default:
      Serial.println("");
      Serial.println("Unknown");
      break;
  }
}


void check_wifi_status_timer_cb(lv_timer_t * timer) {

    // check if wifi is connected
    processWifiState(timer);
}


void connectToWifiNetwork(const String& ssid, const String& password) {
  wl_status_t wifiStatus = WiFi.status();
  debugCurrentWifiStatus();
  if (wifiStatus == WL_IDLE_STATUS || 
      wifiStatus == WL_CONNECT_FAILED || WL_DISCONNECTED) {

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

    while (WiFi.status() != WL_CONNECTED && attempt < 10) {
        delay(500);
        Serial.print(".");
        wl_status_t status = WiFi.status();
        
        if (status == WL_CONNECT_FAILED) {
            Serial.println("Connection failed (wrong password or AP unreachable).");
            break;
        }
        
        if (status == WL_NO_SSID_AVAIL) {
            Serial.println("SSID not found.");
            break;
        }

        attempt++;
    }

    WiFi.mode(WIFI_STA);

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");

        prefs.begin("WLANPrefs", false);
        // store wifi credentials in preferences so they persist after reboot
        prefs.putString("ssid", ssid.c_str());
        // TODO: LIKELY NEED TO ENCRYPT WIFI CREDENTIALS INSTEAD OF STORING IN PLAIN TEXT
        prefs.putString("networkPass", password.c_str());

        prefs.end();

    } else {
        Serial.println("Failed to connect after multiple attempts.");
    }

}


void disconnectFromWifiNetwork() {
  Serial.printf("Disconnecting from %s...\n", WiFi.SSID().c_str());
  WiFi.disconnect();
  delay(200);
  debugCurrentWifiStatus();
  updateWifiWidget(WL_DISCONNECTED);
}

int getWifiSignalStrength() {
    return WiFi.RSSI();
}

void freeNetworks(struct Network* networks) {
    if (networks != NULL) {
        delete[] networks;
    }
}

void processWifiState(lv_timer_t * timer) {
  wl_status_t wifiStatus = WiFi.status();

  debugCurrentWifiStatus();

  switch (wifiStatus) {
    case WL_DISCONNECTED: {

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

        // TODO: update UI for menu
        updateWifiNetworkList(networkCount, networksList);
        WiFi.scanDelete();

        // stop the timer
        lv_timer_del(timer);
        
      } else if (scanResult == WIFI_SCAN_FAILED) {
        // Scan failed
        Serial.println("WiFi scan failed");
             
        // Update UI to show failed connection
        updateWifiWidget(WL_CONNECT_FAILED);

        lv_timer_del(timer);
      }
      else if (millis() - scanStartTime > 15000) {
        // Timeout after 15 seconds
        WiFi.scanDelete();
        Serial.println("WiFi scan timeout");
        
        // Update UI to show timeout
        updateWifiWidget(WL_CONNECT_FAILED);

        lv_timer_del(timer);
      }
      break;
    }
    case WL_CONNECTED: {
          Serial.println("WiFi connected");
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
          
          // Update UI to show connected state
          updateWifiWidget(WL_CONNECTED);
          
          // if wifi is connected stop the timer
          lv_timer_del(timer);
      break;
    }
    default:
    debugCurrentWifiStatus();
  }
}

void check_scan_status_timer_cb(lv_timer_t * timer) {
    debugCurrentWifiStatus();
    // int result = WiFi.scanComplete();

    // if (result == WIFI_SCAN_RUNNING) {
    //     return;
    // }

    //lv_timer_del(timer);

    processWifiState(timer); 

    //updateWifiWidget(currentWifiState);
}

// const char* wifiScanStateToString(WifiScanState state) {
//   switch (state) {
//     case WIFI_IDLE: return "IDLE";
//     case WIFI_SCANNING: return "SCANNING";
//     case WIFI_CONNECTING: return "CONNECTING";
//     case WIFI_CONNECTED: return "CONNECTED";
//     case WIFI_CONNECTION_FAILED: return "CONNECTION FAILED";
//     default: return "UNKNOWN";
//   }
// }



void startWifiScan() {
  wl_status_t wifiStatus = WiFi.status();
  // IF WIFI IS IN CONNECTED STATE AND A SCAN IS ATTEMPTED, SIMPLY REMOVE THE LOADING SPINNER AND ADD THE CONNECTED NETWORK ONLY
  if(wifiStatus == WL_CONNECTED) {

    struct Network* currentNetwork = new struct Network[1];

    currentNetwork[0].num = 1;
    currentNetwork[0].ssid = strdup(WiFi.SSID().c_str()); // Make a copy of the SSID string
    currentNetwork[0].rssi = WiFi.RSSI();
    currentNetwork[0].channel = WiFi.channel();
    currentNetwork[0].encryptionType = "";

    updateWifiNetworkList(1, currentNetwork);
  } else {
      Serial.println("Resetting WiFi before scan");
      WiFi.mode(WIFI_STA); 
      WiFi.disconnect(true); // This clears old configs and resets properly
      delay(200); // Let the WiFi hardware reset

      debugCurrentWifiStatus();

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

}
