#include "preferences_manager.h"

void PreferencesManager::setup_preferences() {
    ScopedPrefs _(prefs, prefsName, true);  // read-only — just loading saved values
    ssid  = prefs.getString("ssid", "");
    pswrd = prefs.getString("networkPass", "");
}

void PreferencesManager::store_wifi_credentials(const char* netwrk, const char* pass) {
    ScopedPrefs _(prefs, prefsName, false);   
    ssid = netwrk;
    pswrd = pass;

    prefs.putString("ssid", ssid);
    prefs.putString("networkPass", pswrd);
}
