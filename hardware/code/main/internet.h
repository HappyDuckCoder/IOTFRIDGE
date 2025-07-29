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

    bool uploadData(const char *data)
    {
        if (!isConnected())
        {
            Serial.println("WiFi chưa kết nối!");
            return false;
        }

        Serial.println("===> Đang gửi dữ liệu cảm biến lên server");

        HTTPClient client;

        // Tạo URL với endpoint /uploadData
        String uploadDataURL = String(serverURL);
        if (!uploadDataURL.endsWith("/"))
        {
            uploadDataURL += "/";
        }
        uploadDataURL += "uploadData";

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

    bool uploadSensorData(float temperature, float humidity, float ch4_ppm, float nh3_ppm, float weight)
    {
        // Tạo JSON string với 5 thông số cảm biến
        String jsonData = "{";
        jsonData += "\"temperature\":" + String(temperature, 1) + ",";
        jsonData += "\"humidity\":" + String(humidity, 1) + ",";
        jsonData += "\"ch4_ppm\":" + String(ch4_ppm, 1) + ",";
        jsonData += "\"nh3_ppm\":" + String(nh3_ppm, 1) + ",";
        jsonData += "\"weight\":" + String(weight, 2) + ",";
        jsonData += "\"timestamp\":" + String(millis());
        jsonData += "}";

        Serial.println("Dữ liệu JSON gửi: " + jsonData);
        return uploadData(jsonData.c_str());
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
