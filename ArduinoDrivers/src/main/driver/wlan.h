#ifndef WLAN_H
#define WLAN_H
#include <Arduino.h>

struct Network {
  int num;
  String ssid;
  int rssi;
  int channel;
  String encryptionType;
};


enum WifiScanState {
  WIFI_IDLE,
  WIFI_SCANNING,
  WIFI_CONNECTING,
  WIFI_CONNECTED,
  WIFI_CONNECTION_FAILED
};

void freeNetworks(struct Network* networks);
void startWifiScan();
void connectToWifiNetwork(const String& ssid, const String& password);
void connectToWifiNetworkBlocking(const String& ssid, const String& password);
void disconnectFromWifiNetwork();
void setWifiState(WifiScanState state);
WifiScanState getWifiState();
int getWifiSignalStrength();
void processWifiState();


void setup_preferences();

#endif