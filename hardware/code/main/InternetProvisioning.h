#ifndef INTERNET_PROVISIONING_H
#define INTERNET_PROVISIONING_H

#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class InternetProvisioning
{
private:
    Preferences prefs;
    WebServer server;
    String ssid;
    String password;
    bool connected;
    String serverBaseURL;
    String EmailUser; 
    String PasswordUser;

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

    bool uploadSensorData(float temp, float humi, bool is_rotted_food, int total_food, int last_open, bool is_saving_mode)
    {
        // Tạo JSON string với 5 thông số cảm biến
        String jsonData = "{";
        jsonData += "\"temp\":" + String(temp, 1) + ",";
        jsonData += "\"humi\":" + String(humi, 1) + ",";
        jsonData += "\"is_rotted_food\":" + String(is_rotted_food ? "true" : "false") + ",";
        jsonData += "\"total_food\":" + String(total_food) + ",";
        jsonData += "\"last_open\":" + String(last_open) + ",";
        jsonData += "\"is_saving_mode\":" + String(is_saving_mode ? "true" : "false") + ",";
        jsonData += "}";

        Serial.println("Dữ liệu JSON gửi: " + jsonData);
        return uploadData(jsonData.c_str(), "/uploadData");
    }

    bool uploadNotification(String message, const char *link)
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

        // String jsonPayload = "{\"message\": " + String(message) + "}";
        String jsonPayload = "{\"message\": \"" + message + "\"}";

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
            Serial.println("Lỗi khi gửi tin nhắn");
        }

        client.end();
        return success;
    }

    bool uploadAuthentification(String email, String password, const char* link)
    {
        
    }

    FridgeData readData(const char *link)
    {
        FridgeData data(0, 0, 0, 0, 0, 0);

        if (!isConnected())
        {
            logNotConnected();
            return data;
        }

        String url = String(serverBaseURL) + String(link);
        Serial.println("Đang gửi yêu cầu GET tới: " + url);

        HTTPClient http;
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode == 200)
        {
            String payload = http.getString();
            Serial.println("Phản hồi từ server:");
            Serial.println(payload);

            StaticJsonDocument<512> doc;
            DeserializationError err = deserializeJson(doc, payload);

            if (!err)
            {
                data.temp = doc["temp"] | 0.0;
                data.humi = doc["humi"] | 0.0;
                data.is_rotted_food = doc["is_rotted_food"] | false;
                data.total_food = doc["total_food"] | 0;
                data.last_open = doc["last_open"] | 0;
                data.is_saving_mode = doc["is_saving_mode"] | false;

                logDebugReceiveData(data);
            }
            else
            {
                Serial.println("Lỗi parse JSON: " + String(err.c_str()));
            }
        }
        else
        {
            Serial.printf("Lỗi kết nối server: %d\n", httpCode);
        }

        http.end();
        return data;
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
        server.on("/", [this]() 
        {
            String page = "<!DOCTYPE html>";
            page += "<html><head>";
            page += "<meta charset='UTF-8'>";
            page += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            page += "<title>IOTsmartFridge WiFi Config</title>";
            page += "<style>";
            page += "* { margin: 0; padding: 0; box-sizing: border-box; }";
            page += "body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }";
            page += ".container { max-width: 500px; margin: 0 auto; background: white; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); overflow: hidden; }";
            page += ".header { background: linear-gradient(135deg, #4CAF50, #45a049); color: white; padding: 30px 20px; text-align: center; }";
            page += ".header h1 { font-size: 24px; margin-bottom: 5px; }";
            page += ".header p { opacity: 0.9; font-size: 14px; }";
            page += ".form-container { padding: 30px; }";
            page += ".section { margin-bottom: 25px; }";
            page += ".section h3 { color: #333; margin-bottom: 15px; font-size: 18px; display: flex; align-items: center; }";
            page += ".wifi-icon, .server-icon, .key-icon { margin-right: 8px; }";
            page += ".wifi-list { max-height: 200px; overflow-y: auto; border: 1px solid #ddd; border-radius: 8px; padding: 10px; background: #f9f9f9; }";
            page += ".wifi-item { display: flex; align-items: center; padding: 12px 8px; margin: 5px 0; background: white; border-radius: 6px; cursor: pointer; transition: all 0.2s; border: 2px solid transparent; }";
            page += ".wifi-item:hover { background: #f0f8ff; border-color: #4CAF50; }";
            page += ".wifi-item input[type='radio'] { margin-right: 12px; }";
            page += ".wifi-name { font-weight: 500; flex-grow: 1; }";
            page += ".wifi-signal { font-size: 12px; color: #666; background: #e9ecef; padding: 2px 8px; border-radius: 12px; }";
            page += ".form-group { margin-bottom: 20px; }";
            page += ".form-group label { display: block; margin-bottom: 8px; color: #555; font-weight: 500; }";
            page += ".form-control { width: 100%; padding: 12px 15px; border: 2px solid #ddd; border-radius: 8px; font-size: 14px; transition: border-color 0.2s; }";
            page += ".form-control:focus { outline: none; border-color: #4CAF50; box-shadow: 0 0 0 3px rgba(76, 175, 80, 0.1); }";
            page += ".btn-submit { width: 100%; padding: 15px; background: linear-gradient(135deg, #4CAF50, #45a049); color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: 500; cursor: pointer; transition: transform 0.2s; }";
            page += ".btn-submit:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(76, 175, 80, 0.3); }";
            page += ".btn-submit:active { transform: translateY(0); }";
            page += "@media (max-width: 480px) { .container { margin: 10px; } .form-container { padding: 20px; } }";
            page += "</style></head>";
            
            page += "<body>";
            page += "<div class='container'>";
            page += "<div class='header'>";
            page += "<h1>IOT Smart Fridge</h1>";
            page += "<p>WiFi & Server Configuration</p>";
            page += "</div>";
            
            page += "<div class='form-container'>";
            page += "<form action='/save' method='POST'>";
            
            // WiFi networks selection
            page += "<div class='section'>";
            page += "<h3><span class='wifi-icon'></span>Select WiFi Network</h3>";
            page += "<div class='wifi-list'>";
            int n = WiFi.scanNetworks();
            if (n > 0) {
                for (int i = 0; i < n; ++i) {
                    int signalStrength = WiFi.RSSI(i);
                    String signalText = "";
                    if (signalStrength > -50) signalText = "Strong";
                    else if (signalStrength > -70) signalText = "Good";
                    else signalText = "Weak";
                    
                    page += "<div class='wifi-item'>";
                    page += "<input type='radio' name='ssid' value='" + WiFi.SSID(i) + "' id='wifi" + String(i) + "'>";
                    page += "<label for='wifi" + String(i) + "' class='wifi-name'>" + WiFi.SSID(i) + "</label>";
                    page += "<span class='wifi-signal'>" + signalText + " (" + String(signalStrength) + "dBm)</span>";
                    page += "</div>";
                }
            } else {
                page += "<div style='text-align: center; color: #666; padding: 20px;'>No WiFi networks found</div>";
            }
            page += "</div>";
            page += "</div>";
            
            // Password input
            page += "<div class='section'>";
            page += "<div class='form-group'>";
            page += "<label for='password'><span class='key-icon'></span>WiFi Password</label>";
            page += "<input type='password' name='pass' id='password' class='form-control' placeholder='Enter WiFi password'>";
            page += "</div>";
            page += "</div>";
            
            // Server URL input
            page += "<div class='section'>";
            page += "<div class='form-group'>";

            page += "<label for='serverurl'><span class='server-icon'></span>Server Address</label>";
            page += "<input type='text' name='serverurl' id='serverurl' class='form-control' value='" + serverBaseURL + "' placeholder='http://192.168.4.11:8888'>";

            page += "<label for='account_user'><span class='account-icon'></span>Account: </label>";
            page += "<input type='text' name='account_user' id='account_user' class='form-control' value='" + EmailUser + "' placeholder='duck@gmail.com'>";

            page += "<label for='password_user'><span class='server-icon'></span>Password: </label>";
            page += "<input type='text' name='password_user' id='password_user' class='form-control' value='" + PasswordUser + "' placeholder='********'>";

            page += "</div>";
            page += "</div>";
            
            page += "<button type='submit' class='btn-submit'>Save Configuration</button>";
            page += "</form>";
            page += "</div>";
            page += "</div>";
            
            page += "<script>";
            page += "document.querySelectorAll('.wifi-item').forEach(item => {";
            page += "  item.addEventListener('click', function() {";
            page += "    this.querySelector('input[type=\"radio\"]').checked = true;"; // gửi hết đống form lên /save
            page += "  });";
            page += "});";
            page += "</script>";
            
            page += "</body></html>";
            
            server.send(200, "text/html", page);
        });

        server.on("/save", [this]() {
            String newSSID = server.arg("ssid");
            String newPASS = server.arg("pass");
            String newServerURL = server.arg("serverurl");
            String newEmail = server.arg("user_email");
            // String newPassword = server.arg("user_password"); // không cần

            saveCredentials(newSSID, newPASS, newServerURL);
            
            String response = "<!DOCTYPE html>";
            response += "<html><head>";
            response += "<meta charset='UTF-8'>";
            response += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            response += "<title>Configuration Saved</title>";
            response += "<style>";
            response += "body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 20px; }";
            response += ".success-container { max-width: 400px; background: white; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); text-align: center; padding: 40px 30px; }";
            response += ".success-icon { font-size: 60px; margin-bottom: 20px; }";
            response += "h1 { color: #4CAF50; margin-bottom: 20px; font-size: 24px; }";
            response += ".info { background: #f8f9fa; border-radius: 8px; padding: 15px; margin: 15px 0; text-align: left; }";
            response += ".info-label { font-weight: bold; color: #333; }";
            response += ".info-value { color: #666; word-break: break-all; }";
            response += ".loading { margin-top: 20px; }";
            response += ".spinner { border: 3px solid #f3f3f3; border-top: 3px solid #4CAF50; border-radius: 50%; width: 30px; height: 30px; animation: spin 1s linear infinite; margin: 0 auto 10px; }";
            response += "@keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }";
            response += "</style></head>";
            
            response += "<body>";
            response += "<div class='success-container'>";
            response += "<div class='success-icon'></div>";
            response += "<h1>Configuration saved successfully!</h1>";
            
            response += "<div class='info'>";
            response += "<div class='info-label'>WiFi Network:</div>";
            response += "<div class='info-value'>" + newSSID + "</div>";
            response += "</div>";
            
            response += "<div class='info'>";
            response += "<div class='info-label'>Server URL:</div>";
            response += "<div class='info-value'>" + newServerURL + "</div>";
            response += "</div>";

            response += "<div class='info'>";
            response += "<div class='info-label'>Email:</div>";
            response += "<div class='info-value'>" + newEmail + "</div>";
            response += "</div>";

            response += "<div class='loading'>";
            response += "<div class='spinner'></div>";
            response += "<p>Đang khởi động lại thiết bị...</p>";
            response += "</div>";
            
            response += "</div>";
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

    void saveCredentials(String newSSID, String newPassword, String newServerURL, String newEmail, String newPassword)
    {
        prefs.putString("ssid", newSSID);
        prefs.putString("password", newPassword);

        if (!newServerURL.isEmpty())
        {
            prefs.putString("serverURL", newServerURL);
            serverBaseURL = newServerURL;
        }

        if (!newAccount.isEmpty())
        {
            prefs.putString("user_email", newAccount);
            EmailUser = newEmail;
        }

        if (!newPassword.isEmpty())
        {
            prefs.putString("user_password", newServerURL);
            Password = newPassword;
        }

        Serial.printf("Đã lưu WiFi: %s\n", newSSID.c_str());
        Serial.printf("Đã lưu Server URL: %s\n", serverBaseURL.c_str());
        Serial.printf("Đã lưu Email: %s\n", newEmail.c_str());
        Serial.printf("Đã lưu Password: %s\n", newPassword.c_str());
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

    void logDebugReceiveData(FridgeData data)
    {
        Serial.println("Dữ liệu đã parse:");
        Serial.print("  Nhiệt độ: ");
        Serial.println(data.temp);
        Serial.print("  Độ ẩm: ");
        Serial.println(data.humi);
        Serial.print("  Có đồ ăn bị hư: ");
        Serial.println(data.is_rotted_food ? "Có" : "Không");
        Serial.print("  Tổng số món ăn: ");
        Serial.println(data.total_food);
        Serial.print("  Lần cuối mở cửa: ");
        Serial.println(data.last_open);
        Serial.print("  Đang ở chế độ tiết kiệm: ");
        Serial.println(data.is_saving_mode ? "Có" : "Không");
    }
};

#endif