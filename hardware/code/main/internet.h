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
    const char *serverBaseURL;
    bool connected;

public:
    Internet(const char *wifiSSID, const char *wifiPassword, const char *baseURL)
    {
        ssid = wifiSSID;
        password = wifiPassword;
        serverBaseURL = baseURL;
        connected = false;
    }

    bool begin()
    {
        Serial.println("Đang kết nối WiFi...");
        WiFi.begin(ssid, password);
        return true;
    }

    void printInfo()
    {
        Serial.print("WiFi đã kết nối! IP: ");
        Serial.print(WiFi.localIP());
        Serial.print(" - ");
        Serial.print("base URL: ");
        Serial.println(serverBaseURL);
    }

    void checking()
    {
        // Nếu chưa kết nối, thử kết nối lại mỗi delay giây
        if (WiFi.status() != WL_CONNECTED)
        {
            connected = false;
            WiFi.begin(ssid, password);
        }
        else
        {
            if (!connected)
            {
                connected = true;
                printInfo();
            }
        }
    }

    bool isConnected()
    {
        return connected && (WiFi.status() == WL_CONNECTED);
    }

    void logNotConnected()
    {
        Serial.println("WiFi chưa kết nối, không thể gửi dữ liệu!");
    }

    void testUploadingInMain()
    {
        int dummyData = random(10, 100);
        bool success = uploadTestData(dummyData, "/uploadTestData");

        if (success)
        {
            Serial.println("Gửi test_data thành công!");
        }
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

    bool uploadFile(const char *filename, const char *uploadURL)
    {
        if (!isConnected())
        {
            logNotConnected();
            return false;
        }

        File file = SPIFFS.open(filename, FILE_READ);
        if (!file)
        {
            Serial.println("FILE KHÔNG TỒN TẠI!");
            return false;
        }

        Serial.println("===> Đang upload file lên server");

        HTTPClient client;
        client.begin(uploadURL);
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
            Serial.println("==================== Kết thúc ====================");
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

    bool uploadData(const char *data, const char *uploadURL)
    {
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
