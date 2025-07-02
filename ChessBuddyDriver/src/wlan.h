#ifndef WLAN_H
#define WLAN_H
#include <Arduino.h>
#include <WiFi.h>
#include <lvgl.h>

struct Network {
  int num;
  String ssid;
  int rssi;
  int channel;
  String encryptionType;
};

void freeNetworks(struct Network* networks);
void startWifiScan();
void connectToWifiNetwork(const String& ssid, const String& password);
void connectToWifiNetworkBlocking(const String& ssid, const String& password);
void disconnectFromWifiNetwork();
int getWifiSignalStrength();
void processWifiState(lv_timer_t * timer);


void setup_preferences();

#endif