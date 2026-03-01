#include "wlan.h"
#include "gui/gui_gateway.h"

WLAN::WLAN(GUI_GATEWAY& gui_gateway, PreferencesManager& prefsManager) : gui_gateway(gui_gateway), preferencesManager(prefsManager) {}

void WLAN::setup_wlan() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  auto [ssid, password] = preferencesManager.get_wifi_credentials();

  // attempt to connect to network
  if(ssid.length() != 0 && password.length() !=0) {
    connectToWifiNetwork(ssid, password);
  }
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

void WLAN::debugCurrentWifiStatus() {

  bool isconn = WiFi.isConnected();
  Serial.print("Is wifi connected: ");
  Serial.println(isconn);  // Print numeric value

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


void WLAN::check_wifi_status_timer_cb(lv_timer_t * timer) {
    WLAN* wlan = static_cast<WLAN*>(lv_timer_get_user_data(timer));
    if (!wlan) return;

    // check if wifi is connected
    wlan->processWifiState(timer);
}


void WLAN::connectToWifiNetwork(const String& ssid, const String& password) {
  wl_status_t wifiStatus = WiFi.status();
  debugCurrentWifiStatus();
  if (wifiStatus == WL_IDLE_STATUS || 
      wifiStatus == WL_CONNECT_FAILED || wifiStatus == WL_DISCONNECTED) {

    scanStartTime = millis(); // Reuse for connection timeout
    attempt = 1;
    
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.printf("Connecting to %s...\n", ssid.c_str());
    
    // Update UI to show connecting state
    //showWifiConnecting(ssid);

    // reset attempt count
    attempt = 1;
    lv_timer_create(check_wifi_status_timer_cb, 500, this);
  }
}


void WLAN::connectToWifiNetworkBlocking(const String& ssid, const String& password) {
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
        preferencesManager.store_wifi_credentials(ssid.c_str(), password.c_str());

    } else {
        Serial.println("Failed to connect after multiple attempts.");
    }

}


void WLAN::disconnectFromWifiNetwork() {
  Serial.printf("Disconnecting from %s...\n", WiFi.SSID().c_str());
  WiFi.disconnect();
  delay(200);
  debugCurrentWifiStatus();
  gui_gateway.request_wifi_icon_update(WL_DISCONNECTED);
}

int WLAN::getWifiSignalStrength() {
    return WiFi.RSSI();
}

void WLAN::freeNetworks(struct cNetwork* networks) {
    if (networks != NULL) {
        delete[] networks;
    }
}

const char* WLAN::getNetworkEncryptionType(wifi_auth_mode_t type) {
  switch (type) {
    case WIFI_AUTH_OPEN:
      return "open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA+WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-EAP";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2+WPA3";
    case WIFI_AUTH_WAPI_PSK:
      return "WAPI";
    default:
      return "unknown";
  }
}


void WLAN::processWifiState(lv_timer_t * timer) {

  wl_status_t wifiStatus = WiFi.status();
  debugCurrentWifiStatus();
  
  switch (wifiStatus) {
    case WL_CONNECTED:
      Serial.println("WiFi connected!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      gui_gateway.request_wifi_icon_update(WL_CONNECTED);
      lv_timer_del(timer);
      break;
      
    case WL_CONNECT_FAILED:
      Serial.println("WiFi connection failed (wrong password)");
      gui_gateway.request_wifi_icon_update(WL_CONNECT_FAILED);
      lv_timer_del(timer);
      break;
      
    case WL_NO_SSID_AVAIL:
      Serial.println("WiFi SSID not found");
      gui_gateway.request_wifi_icon_update(WL_CONNECT_FAILED);
      lv_timer_del(timer);
      break;
      
    case WL_DISCONNECTED:
    case WL_IDLE_STATUS:
      attempt++;
      // stop timer after a certain number of wifi status checks failed
      if(attempt > max_attempts) {
        Serial.println("Wifi connection failed (maximum attempts exceeded)");
        // Update UI to show failed connection
        gui_gateway.request_wifi_icon_update(WL_CONNECT_FAILED);

        lv_timer_del(timer);
        break;
      }

      // Still connecting, check timeout
      if (millis() - scanStartTime > 30000) {
        Serial.println("WiFi connection timeout");
        WiFi.disconnect();
        gui_gateway.request_wifi_icon_update(WL_CONNECT_FAILED);
        lv_timer_del(timer);
      }
      break;
      
    default:
      Serial.printf("Unexpected WiFi status: %d\n", wifiStatus);
      break;
  }
}

void WLAN::checkScanStatus(lv_timer_t* timer) {
  int scanResult = WiFi.scanComplete();
  Serial.print("Scan Status (int): ");
  Serial.println(scanResult);

  switch(scanResult) {
    case(WIFI_SCAN_FAILED):
        // Scan failed
        Serial.println("WiFi scan failed");
        if(failedScans > max_scans) {
          // Update UI to show failed connection
          gui_gateway.request_wifi_icon_update(WL_CONNECT_FAILED);

          lv_timer_del(timer);
        }
        failedScans++;
      return;
    case(WIFI_SCAN_RUNNING):
      return;
    default:
    networkCount = scanResult;
    //Free previous results if they exist
    if (networksList != NULL) {
      freeNetworks(networksList);
      networksList = NULL;
    }
    
    networksList = new struct cNetwork[networkCount];
    for (int i = 0; i < networkCount; ++i) {
      networksList[i].num = i + 1;
      networksList[i].ssid = WiFi.SSID(i).c_str();
      networksList[i].rssi = WiFi.RSSI(i);
      networksList[i].channel = WiFi.channel(i);
      networksList[i].encryptionType = getNetworkEncryptionType(WiFi.encryptionType(i));
      delay(10);
    }

    // TODO: update UI for menu
    gui_gateway.request_wifi_list_update(networkCount, networksList);
    WiFi.scanDelete();

    // stop the timer
    lv_timer_del(timer);
  }
}

void WLAN::check_scan_status_timer_cb(lv_timer_t * timer) {
    WLAN* wlan = static_cast<WLAN*>(lv_timer_get_user_data(timer));
    if (!wlan) return;

    wlan->checkScanStatus(timer); 

    //updateWifiWidget(currentWifiState);
}

void WLAN::startWifiScan() {
  wl_status_t wifiStatus = WiFi.status();
  // IF WIFI IS IN CONNECTED STATE AND A SCAN IS ATTEMPTED, SIMPLY REMOVE THE LOADING SPINNER AND ADD THE CONNECTED NETWORK ONLY
  if(wifiStatus == WL_CONNECTED) {

    if (currentNetwork) {
      freeNetworks(currentNetwork);
      currentNetwork = NULL;
    }
    currentNetwork = new struct cNetwork[1];

    currentNetwork[0].num = 1;
    currentNetwork[0].ssid = WiFi.SSID();
    currentNetwork[0].rssi = WiFi.RSSI();
    currentNetwork[0].channel = WiFi.channel();
    currentNetwork[0].encryptionType = getNetworkEncryptionType(WiFi.encryptionType(0));

    gui_gateway.request_wifi_list_update(1, currentNetwork);
  } else {
      Serial.println("Resetting WiFi before scan");
      WiFi.mode(WIFI_STA); 
      WiFi.disconnect(true); // This clears old configs and resets properly
      delay(200); // Let the WiFi hardware reset

      debugCurrentWifiStatus();

      scanStartTime = millis();
      failedScans = 0;
      
      // Start asynchronous WiFi scan
      WiFi.scanNetworks(true); // true = async mode
      
      Serial.println("WiFi scan started");
      
      lv_timer_create(check_scan_status_timer_cb, 500, this);
      // TODO: update UI to show wifi scanning animation in lvgl
      //updateWifiScanningUI(true);

  }

}
