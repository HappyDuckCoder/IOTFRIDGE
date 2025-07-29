#ifndef INTERNET_PROVISIONING_H
#define INTERNET_PROVISIONING_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

class InternetProvisioning {
private:
    Preferences prefs;
    WebServer server;
    String ssid;
    String password;
    bool connected;

public:
    InternetProvisioning();
    void begin();
    void handleClient();
    bool isConnected();
    String getSSID();
    String getPassword();

private:
    void startAPMode();
    void startWebPortal();
    void scanNetworks();
    void saveCredentials(String newSSID, String newPassword);
    bool connectToSavedWiFi();
};

#endif
