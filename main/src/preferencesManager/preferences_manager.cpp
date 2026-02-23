#include "preferences_manager.h"

void PreferencesManager::setup_preferences() {
    ScopedPrefs _(prefs, prefsName, true);  // read-only — just loading saved values
    ssid  = prefs.getString("ssid", "");
    pswrd = prefs.getString("networkPass", "");
}
