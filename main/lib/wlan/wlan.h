#ifndef WLAN_H
#define WLAN_H

#include <Arduino.h>
#include <WiFi.h>
#include <lvgl.h>
#include "gui_gateway.h"
#include <Preferences.h>
#include "FT6336U.h"
#include "Main_Definitions.h"
#include "User_Setup.h"
#include <WiFi.h>

class WLAN {
private:
  Preferences prefs;

  int networkCount = 0;

  unsigned long scanStartTime = 0;

  const int max_attempts = 10;
  int attempt = 1;

  int failedScans = 0;

  void debugCurrentWifiStatus();
public:
  WLAN();

  struct cNetwork {
    int num;
    String ssid;
    int rssi;
    int channel;
    const char* encryptionType;
  };

  cNetwork* networksList = NULL;
  cNetwork* currentNetwork = NULL;

  void freeNetworks(struct cNetwork* networks);
  void startWifiScan();
  void connectToWifiNetwork(const String& ssid, const String& password);
  void connectToWifiNetworkBlocking(const String& ssid, const String& password);
  void disconnectFromWifiNetwork();
  int getWifiSignalStrength();
  void processWifiState(lv_timer_t * timer);

  // timers and other static lvgl object references
  static void check_wifi_status_timer_cb(lv_timer_t * timer);
  static void check_scan_status_timer_cb(lv_timer_t * timer);

  void setup_preferences();
};

#endif