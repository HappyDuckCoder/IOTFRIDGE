#ifndef SPIFF_H
#define SPIFF_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

class Spiff
{
public:
    Spiff() {}

    bool begin(bool formatOnFail = true)
    {
        if (!SPIFFS.begin(formatOnFail))
        {
            return false;
        }
        return true;
    }

    bool writeFile(const char *path, const String &data)
    {
        File file = SPIFFS.open(path, FILE_WRITE);
        if (!file)
        {
            Serial.printf("Không thể mở file để ghi: %s\n", path);
            return false;
        }
        file.print(data);
        file.close();
        Serial.printf("Đã ghi vào file: %s\n", path);
        return true;
    }

    String readFile(const char *path)
    {
        File file = SPIFFS.open(path, FILE_READ);
        if (!file || file.isDirectory())
        {
            Serial.printf("Không thể mở file để đọc: %s\n", path);
            return "";
        }

        String content;
        while (file.available())
        {
            content += (char)file.read();
        }
        file.close();
        Serial.printf("Đã đọc file: %s\n", path);
        return content;
    }

    bool deleteFile(const char *path)
    {
        if (SPIFFS.remove(path))
        {
            Serial.printf("Đã xóa file: %s\n", path);
            return true;
        }
        else
        {
            Serial.printf("Không thể xóa file: %s\n", path);
            return false;
        }
    }

    void deleteAllFiles(const char *dir = "/")
    {
        File root = SPIFFS.open(dir);
        if (!root || !root.isDirectory())
        {
            Serial.println("Không thể mở thư mục SPIFFS.");
            return;
        }

        File file = root.openNextFile();
        while (file)
        {
            const char *fileName = file.name();
            file.close(); // đóng file trước khi xóa
            if (SPIFFS.remove(fileName))
            {
                Serial.printf("Đã xóa file: %s\n", fileName);
            }
            else
            {
                Serial.printf("Không thể xóa file: %s\n", fileName);
            }
            file = root.openNextFile();
        }

        Serial.println("Hoàn tất xóa toàn bộ file.");
    }

    bool exists(const char *path)
    {
        return SPIFFS.exists(path);
    }

    void listFiles(const char *dir = "/")
    {
        Serial.println("Danh sách file SPIFFS:");
        File root = SPIFFS.open(dir);
        if (!root || !root.isDirectory())
        {
            Serial.println("Thư mục không hợp lệ.");
            return;
        }

        File file = root.openNextFile();
        while (file)
        {
            Serial.print("");
            Serial.print(file.name());
            Serial.print(" (");
            Serial.print(file.size());
            Serial.println(" bytes)");
            file = root.openNextFile();
        }
    }
};

#endif
