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

        Serial.println("===> Đang upload file lên server Node.js");

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