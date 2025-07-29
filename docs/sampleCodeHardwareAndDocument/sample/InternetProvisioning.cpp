#include "InternetProvisioning.h"

InternetProvisioning::InternetProvisioning() : server(80) {
    connected = false;
}

void InternetProvisioning::begin() {
    prefs.begin("wifi", false);

    ssid = prefs.getString("ssid", "");
    password = prefs.getString("password", "");

    if (!ssid.isEmpty()) {
        Serial.printf("Đang kết nối WiFi đã lưu: %s\n", ssid.c_str());
        if (connectToSavedWiFi()) {
            connected = true;
            Serial.println("Kết nối thành công!");
            return;
        }
    }

    Serial.println("Không thể kết nối, bật chế độ AP...");
    startAPMode();
}

bool InternetProvisioning::connectToSavedWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    return WiFi.status() == WL_CONNECTED;
}

void InternetProvisioning::startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP_Config", "12345678");
    Serial.println("AP Mode SSID: ESP_Config | Pass: 12345678");
    Serial.println("Truy cập http://192.168.4.1 để cấu hình WiFi");

    startWebPortal();
}

void InternetProvisioning::startWebPortal() {
    server.on("/", [this]() {
        String page = "<html><body><h2>Chọn WiFi</h2><form action='/save'>";
        int n = WiFi.scanNetworks();
        for (int i = 0; i < n; ++i) {
            page += "<input type='radio' name='ssid' value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + "dBm)<br>";
        }
        page += "Password: <input type='password' name='pass'><br><input type='submit' value='Lưu'></form></body></html>";
        server.send(200, "text/html", page);
    });

    server.on("/save", [this]() {
        String newSSID = server.arg("ssid");
        String newPASS = server.arg("pass");

        saveCredentials(newSSID, newPASS);
        server.send(200, "text/html", "<html><body><h2>Lưu thành công! ESP sẽ khởi động lại...</h2></body></html>");
        delay(2000);
        ESP.restart();
    });

    server.begin();
}

void InternetProvisioning::handleClient() {
    server.handleClient();
}

void InternetProvisioning::saveCredentials(String newSSID, String newPassword) {
    prefs.putString("ssid", newSSID);
    prefs.putString("password", newPassword);
    Serial.printf("Đã lưu WiFi: %s\n", newSSID.c_str());
}

bool InternetProvisioning::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String InternetProvisioning::getSSID() {
    return ssid;
}

String InternetProvisioning::getPassword() {
    return password;
}


/* sample usage in main.cpp
#include "InternetProvisioning.h"

InternetProvisioning wifiManager;

void setup() {
    Serial.begin(115200);
    wifiManager.begin();
}

void loop() {
    wifiManager.handleClient();

    if (wifiManager.isConnected()) {
        // Code kết nối server...
    }
}

*/