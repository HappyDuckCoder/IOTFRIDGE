#ifndef INTERNET_H
#define INTERNET_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

class Internet
{
private:
    const char *ssid;
    const char *password;
    const char *serverURL;
    bool connected;
    TaskHandle_t wifiTaskHandle;

    static void wifiConnectTask(void *pvParameters)
    {
        Internet *internet = (Internet *)pvParameters;
        internet->connectWiFi();
    }

    void connectWiFi()
    {
        Serial.println("Đang kết nối WiFi...");
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED)
        {
            vTaskDelay(500);
            Serial.print(".");
        }

        connected = true;
        Serial.println();
        Serial.print("WiFi đã kết nối! IP: ");
        Serial.println(WiFi.localIP());

        // Duy trì kết nối
        while (true)
        {
            if (WiFi.status() != WL_CONNECTED)
            {
                connected = false;
                Serial.println("Mất kết nối WiFi, đang thử kết nối lại...");
                WiFi.begin(ssid, password);
                while (WiFi.status() != WL_CONNECTED)
                {
                    vTaskDelay(500);
                    Serial.print(".");
                }
                connected = true;
                Serial.println("\nKết nối WiFi đã được khôi phục!");
            }
            vTaskDelay(1000);
        }
    }

public:
    Internet(const char *wifiSSID, const char *wifiPassword, const char *uploadURL)
    {
        ssid = wifiSSID;
        password = wifiPassword;
        serverURL = uploadURL;
        connected = false;
        wifiTaskHandle = NULL;
    }

    void begin()
    {
        xTaskCreate(wifiConnectTask, "wifi_connect", 4096, this, 1, &wifiTaskHandle);
    }

    bool isConnected()
    {
        return connected && (WiFi.status() == WL_CONNECTED);
    }

    bool uploadFile(const char *filename)
    {
        if (!isConnected())
        {
            Serial.println("WiFi chưa kết nối!");
            return false;
        }

        File file = SPIFFS.open(filename, FILE_READ);
        if (!file)
        {
            Serial.println("FILE KHÔNG TỒN TẠI!");
            return false;
        }

        Serial.println("===> Đang upload file lên server python");

        HTTPClient client;
        client.begin(serverURL);
        client.addHeader("Content-Type", "audio/wav");

        int httpResponseCode = client.sendRequest("POST", &file, file.size());
        Serial.print("Mã phản hồi HTTP: ");
        Serial.println(httpResponseCode);

        bool success = false;
        if (httpResponseCode == 200)
        {
            String response = client.getString();
            Serial.println("==================== Phiên âm ====================");
            Serial.println(response);
            Serial.println("====================   Kết thúc   ====================");
            success = true;
        }
        else
        {
            Serial.println("Lỗi upload");
        }

        file.close();
        client.end();
        return success;
    }

    bool uploadData(const char* data, const char* uploadLink)
    {
        if (!isConnected())
        {
            Serial.println("WiFi chưa kết nối!");
            return false;
        }
        Serial.println("===> Đang gửi dữ liệu cảm biến lên server");
        HTTPClient client;

        // Tạo URL với endpoint
        String uploadDataURL = String(serverURL) + String(uploadLink);
        client.begin(uploadDataURL);
        client.addHeader("Content-Type", "application/json");

        int httpResponseCode = client.POST(data);
        Serial.print("Mã phản hồi HTTP: ");
        Serial.println(httpResponseCode);

        bool success = false;
        if (httpResponseCode == 200)
        {
            String response = client.getString();
            Serial.println("==================== Phản hồi từ server ====================");
            Serial.println(response);
            Serial.println("====================   Kết thúc   ====================");
            success = true;
        }
        else if (httpResponseCode > 0)
        {
            String response = client.getString();
            Serial.println("Lỗi từ server:");
            Serial.println(response);
        }
        else
        {
            Serial.println("Lỗi kết nối tới server");
        }

        client.end();
        return success;
    }

    bool uploadSensorData(float temp, float humi, bool is_rotted_food, int total_food, int last_open)
    {
        // Tạo JSON string với 5 thông số cảm biến
        String jsonData = "{";
        jsonData += "\"temperature\":" + String(temp, 1) + ",";
        jsonData += "\"humidity\":" + String(humi, 1) + ",";
        jsonData += "\"is_rotted_food\":" + String(is_rotted_food ? "true" : "false") + ",";
        jsonData += "\"total_food\":" + String(total_food) + ",";
        jsonData += "\"last_open\":" + String(last_open) + ",";
        jsonData += "\"timestamp\":" + String(millis());
        jsonData += "}";

        Serial.println("Dữ liệu JSON gửi: " + jsonData);
        return uploadData(jsonData.c_str(), "/uploadData");
    }

    String getLocalIP()
    {
        if (isConnected())
        {
            return WiFi.localIP().toString();
        }
        return "Không kết nối";
    }
};

#endif