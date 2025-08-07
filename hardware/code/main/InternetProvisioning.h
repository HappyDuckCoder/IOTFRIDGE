#ifndef INTERNET_PROVISIONING_H
#define INTERNET_PROVISIONING_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

class InternetProvisioning
{
private:
    Preferences prefs;
    WebServer server;
    String ssid;
    String password;
    bool connected;
    String serverBaseURL;

public:
    InternetProvisioning() : server(80), serverBaseURL("")
    {
        connected = false;
    }

    void setServerBaseURL(String baseURL) 
    {
        serverBaseURL = baseURL;
    }

    bool begin()
    {
        prefs.begin("wifi", false);

        ssid = prefs.getString("ssid", "");
        password = prefs.getString("password", "");
        serverBaseURL = prefs.getString("serverURL", "");

        if (!ssid.isEmpty())
        {
            Serial.printf("Đang kết nối WiFi đã lưu: %s\n", ssid.c_str());
            if (connectToSavedWiFi())
            {
                connected = true;
                Serial.println("Kết nối thành công!");
                return true;
            }
        }

        Serial.println("Không thể kết nối, bật chế độ AP...");
        startAPMode();
        return true;
    }

    void handleClient()
    {
        server.handleClient();
    }

    bool isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }

    String getSSID()
    {
        return ssid;
    }

    String getPassword()
    {
        return password;
    }

    bool uploadTestData(int test_data, const char *link) 
    {
        if (!isConnected())
        {
            logNotConnected();
            return false;
        }

        HTTPClient client;

        String uploadURL = String(serverBaseURL) + String(link);
        client.begin(uploadURL);

        Serial.print("đang gửi test đến ");
        Serial.println(uploadURL);

        client.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"test_data\": " + String(test_data) + "}";

        Serial.println("Đang gửi test_data lên server: " + jsonPayload);

        int httpResponseCode = client.POST(jsonPayload);

        Serial.print("Mã phản hồi HTTP: ");
        Serial.println(httpResponseCode);

        bool success = false;
        if (httpResponseCode == 200)
        {
            String response = client.getString();
            Serial.println("Phản hồi từ server đối với test_data:");
            Serial.println(response);
            success = true;
        }
        else
        {
            Serial.println("Lỗi khi gửi dữ liệu test_data");
        }

        client.end();
        return success;
    }

    bool uploadFile(const char *filename, const char *uploadPath)
    {
        Serial.println("Bắt đầu up file");

        if (!isConnected())
        {
            logNotConnected();
            return false;
        }

        // Kiểm tra file tồn tại
        if (!SPIFFS.exists(filename))
        {
            Serial.printf("FILE KHÔNG TỒN TẠI: %s\n", filename);
            return false;
        }

        File file = SPIFFS.open(filename, FILE_READ);
        if (!file)
        {
            Serial.println("KHÔNG THỂ MỞ FILE!");
            return false;
        }

        // Tạo full URL
        String fullURL = String(serverBaseURL) + String(uploadPath);
        Serial.printf("Upload URL: %s\n", fullURL.c_str());
        Serial.printf("File size: %d bytes\n", file.size());

        HTTPClient client;
        client.begin(fullURL);
        client.addHeader("Content-Type", "audio/pcm");
        client.setTimeout(10000);

        int httpResponseCode = client.sendRequest("POST", &file, file.size());
        Serial.printf("Mã phản hồi HTTP: %d\n", httpResponseCode);

        bool success = false;
        if (httpResponseCode == 200)
        {
            String response = client.getString();
            Serial.println(response);
            success = true;
        }
        else if (httpResponseCode > 0)
        {
            String response = client.getString();
            Serial.printf("Lỗi HTTP %d: %s\n", httpResponseCode, response.c_str());
        }
        else
        {
            Serial.printf("Lỗi kết nối: %d\n", httpResponseCode);
        }

        if (file)
        {
            Serial.println("Reset file handle");
            file = File();
            Serial.flush();
        }

        client.end();
        Serial.println("Hoàn thành up file");
        return success;
    }

    bool uploadData(const char *data, const char *link)
    {
        if (!isConnected())
        {
            Serial.println("WiFi chưa kết nối!");
            return false;
        }

        Serial.println("===> Đang gửi dữ liệu cảm biến lên server");

        HTTPClient client;

        // Tạo URL với endpoint /uploadData
        String uploadDataURL = String(serverBaseURL) + String(link);

        client.begin(uploadDataURL);
        client.addHeader("Content-Type", "application/json");

        int httpResponseCode = client.POST(data);
        Serial.print("Mã phản hồi HTTP: ");
        Serial.println(httpResponseCode);

        bool success = false;
        if (httpResponseCode == 200)
        {
            String response = client.getString();
            Serial.println("Phản hồi từ server:");
            Serial.println(response);
            success = true;
        }
        else
        {
            Serial.println("Lỗi kết nối tới server");
        }

        client.end();
        return success;
    }

    bool uploadSensorData(float temp, float humi, bool is_rotted_food, int total_food, int last_open, bool is_auto_mode)
    {
        // Tạo JSON string với 5 thông số cảm biến
        String jsonData = "{";
        jsonData += "\"temperature\":" + String(temp, 1) + ",";
        jsonData += "\"humidity\":" + String(humi, 1) + ",";
        jsonData += "\"spoiledFood\":" + String(is_rotted_food ? "true" : "false") + ",";
        jsonData += "\"foodCount\":" + String(total_food) + ",";
        jsonData += "\"lastDateEntry\":" + String(last_open) + ",";
        jsonData += "\"isAutoMode\":" + String(is_auto_mode) + ",";
        jsonData += "}";

        Serial.println("Dữ liệu JSON gửi: " + jsonData);
        return uploadData(jsonData.c_str(), "/uploadData");
    }

    bool uploadNotification(String message, const char* link)
    {
        if (!isConnected())
        {
            Serial.println("WiFi chưa kết nối!");
            return false;
        }

        Serial.println("===> Đang gửi tin nhắn lên server");

        HTTPClient client;

        String uploadURL = String(serverBaseURL) + String(link);
        client.begin(uploadURL);
        client.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"message\": " + String(message) + "}";

        Serial.println("Đang gửi tin nhắn đến server");

        int httpResponseCode = client.POST(jsonPayload);

        Serial.print("Mã phản hồi HTTP: ");
        Serial.println(httpResponseCode);

        bool success = false;
        if (httpResponseCode == 200)
        {
            String response = client.getString();
            Serial.println("Phản hồi từ server đối với test_data:");
            Serial.println(response);
            success = true;
        }
        else
        {
            Serial.println("Lỗi khi gửi dữ liệu test_data");
        }

        client.end();
        return success;
    } 

private:
    void startAPMode()
    {
        WiFi.mode(WIFI_AP);
        WiFi.softAP("IOTSmartFridge", "13571357");
        Serial.println("AP Mode SSID: IOTSmartFridge | Pass: 13571357");
        Serial.println("Truy cập http://" + WiFi.softAPIP().toString() + " để cấu hình WiFi");

        startWebPortal();
    }

    void startWebPortal()
    {
        server.on("/", [this]() {
            String page = "<html><head><title>IOTsmartFridge WiFi Config</title></head><body>";
            page += "<h2>Wifi & Server Configuration</h2>";
            page += "<form action='/save' method='POST'>";
            
            // WiFi networks selection
            page += "<h3>Choose your WiFi:</h3>";
            int n = WiFi.scanNetworks();
            for (int i = 0; i < n; ++i) {
                page += "<input type='radio' name='ssid' value='" + WiFi.SSID(i) + "'> " + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + "dBm)<br>";
            }
            
            // Password input
            page += "<br><label>Password WiFi:</label><br>";
            page += "<input type='password' name='pass' style='width:300px; padding:5px;'><br><br>";
            
            // Server URL input
            page += "<label>Base Server URL:</label><br>";
            page += "<input type='text' name='serverurl' value='" + serverBaseURL + "' style='width:300px; padding:5px;' placeholder='http://192.168.4.11:8888'><br><br>";
            
            page += "<input type='submit' value='Save' style='padding:10px 20px; background:#4CAF50; color:white; border:none; cursor:pointer;'>";
            page += "</form></body></html>";
            
            server.send(200, "text/html", page);
        });

        server.on("/save", [this]() {
            String newSSID = server.arg("ssid");
            String newPASS = server.arg("pass");
            String newServerURL = server.arg("serverurl");

            saveCredentials(newSSID, newPASS, newServerURL);
            
            String response = "<html><head><title>Saved</title></head><body>";
            response += "<h2>Save config successfully!</h2>";
            response += "<p>SSID: " + newSSID + "</p>";
            response += "<p>Server URL: " + newServerURL + "</p>";
            response += "<p>reseting...</p>";
            response += "</body></html>";
            
            server.send(200, "text/html", response);
            delay(2000);
            ESP.restart();
        });

        server.begin();
    }

    void scanNetworks()
    {
        WiFi.scanNetworks();
    }

    void saveCredentials(String newSSID, String newPassword, String newServerURL)
    {
        prefs.putString("ssid", newSSID);
        prefs.putString("password", newPassword);
        
        if (!newServerURL.isEmpty()) {
            prefs.putString("serverURL", newServerURL);
            serverBaseURL = newServerURL;
        }
        
        Serial.printf("Đã lưu WiFi: %s\n", newSSID.c_str());
        Serial.printf("Đã lưu Server URL: %s\n", serverBaseURL.c_str());
    }

    bool connectToSavedWiFi()
    {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
        {
            delay(500);
            Serial.print(".");
        }
        Serial.println();
        return WiFi.status() == WL_CONNECTED;
    }

    void logNotConnected()
    {
        Serial.println("WiFi chưa kết nối, không thể gửi dữ liệu!");
    }
};

#endif