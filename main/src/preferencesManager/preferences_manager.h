#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Arduino.h>
#include "Main_Definitions.h"
#include "User_Setup.h"
#include <Preferences.h>

class PreferencesManager {
private:
    Preferences prefs;
    const char* prefsName = "MainPrefs";

    String ssid;
    String pswrd;

    // RAII guard: opens preferences on construction, closes on destruction.
    // Usage: ScopedPrefs _(prefs, prefsName);          // read-write
    //        ScopedPrefs _(prefs, prefsName, true);    // read-only
    struct ScopedPrefs {
        Preferences& prefs;
        ScopedPrefs(Preferences& p, const char* name, bool readOnly = false) : prefs(p) {
            p.begin(name, readOnly);
        }
        ~ScopedPrefs() { prefs.end(); }
    };

public:
    void setup_preferences();

    std::pair<String, String> get_wifi_credentials() { return {ssid, pswrd}; }
};


#endif /* PREFERENCES_MANAGER_H */
