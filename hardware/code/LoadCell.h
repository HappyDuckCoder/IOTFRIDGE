#ifndef LOADCELL_H
#define LOADCELL_H

#include "constant.h"

struct LoadCellData
{
    float weight;
    bool isValid;
    unsigned long lastReadTime;

    LoadCellData()
    {
        weight = 0.0;
        isValid = false;
        lastReadTime = 0;
    }
};

class LoadCellSensor
{
private:
    HX711 scale;
    int doutPin;
    int sckPin;
    unsigned long readInterval;
    LoadCellData data;

    float weightThreshold;
    float lastStableWeight;

    bool doorOpen = false;
    unsigned long doorOpenMillis = 0;

public:
    LoadCellSensor(int dout, int sck, unsigned long interval = 2000, float threshold = 50.0)
        : doutPin(dout), sckPin(sck), readInterval(interval), weightThreshold(threshold)
    {
        lastStableWeight = 0.0;
    }

    bool begin()
    {
        pinMode(DOOR_BUTTON_PIN, INPUT_PULLUP);
        scale.begin(doutPin, sckPin);
        scale.set_scale(); // Cần hiệu chỉnh nếu có giá trị scale
        scale.tare();
        lastStableWeight = scale.get_units(10);
        return true;
    }

    void handleRead()
    {
        unsigned long currentTime = millis();
        if (currentTime - data.lastReadTime >= readInterval)
        {
            float newWeight = scale.get_units(10);
            data.weight = newWeight;
            data.isValid = true;
            data.lastReadTime = currentTime;
        }
    }

    void handleDoorTracking()
    {
        bool isClosed = digitalRead(DOOR_BUTTON_PIN) == LOW;

        if (!isClosed) // cửa đang mở
        {
            if (!doorOpen)
            {
                doorOpen = true;
                doorOpenMillis = millis();
                Serial.println("Cửa tủ đã mở");
            }
        }
        else // cửa đã đóng
        {
            if (doorOpen)
            {
                doorOpen = false;
                Serial.println("Cửa đã đóng lại");

                handleRead(); // đọc lại trọng lượng
                float diff = data.weight - lastStableWeight;

                if (abs(diff) > weightThreshold)
                {
                    Serial.printf("Trọng lượng thay đổi: %.2f g\n", diff);
                    if (diff > 0)
                        Serial.println("Yêu cầu nhập thông tin thực phẩm vừa thêm vào.");
                    else
                        Serial.println("Có thực phẩm được lấy ra.");
                }

                lastStableWeight = data.weight;
            }
        }
    }

    LoadCellData getData() const
    {
        return data;
    }

    void log() const
    {
        if (data.isValid)
        {
            Serial.printf("LoadCell - Trọng lượng hiện tại: %.2f g\n", data.weight);
        }
        else
        {
            Serial.println("LoadCell - Lỗi đọc dữ liệu");
        }
    }

    void setThreshold(float threshold)
    {
        weightThreshold = threshold;
    }

    bool isInDanger() const
    {
        return data.isValid && abs(data.weight - lastStableWeight) > weightThreshold && doorOpen && millis() - doorOpenMillis < MAX_DOOR_OPEN_TIME;
    }
};

#endif // LOADCELL_H
