#ifndef DHT_H
#define DHT_H

#include <Arduino.h>
#include <DHT.h>

struct DHTData
{
    float temperature;
    float humidity;
    bool isValid;
    unsigned long lastReadTime;

    DHTData()
    {
        temperature = 0.0;
        humidity = 0.0;
        isValid = false;
        lastReadTime = 0;
    }
};

class DHTSensor
{
private:
    DHT dht;
    int pin;
    int type;
    DHTData data;
    unsigned long readInterval;

    // Ngưỡng cảnh báo
    float tempMin, tempMax;
    float humidityMin, humidityMax;

public:
    DHTSensor(int dhtPin, int dhtType, unsigned long interval = 2000)
        : dht(dhtPin, dhtType), pin(dhtPin), type(dhtType), readInterval(interval)
    {
        // Thiết lập ngưỡng mặc định phù hợp với DHT11
        tempMin = 18.0;     // Nhiệt độ tối thiểu (°C)
        tempMax = 30.0;     // Nhiệt độ tối đa (°C)
        humidityMin = 40.0; // Độ ẩm tối thiểu (%)
        humidityMax = 70.0; // Độ ẩm tối đa (%)
    }

    bool begin()
    {
        dht.begin();
        delay(1000);
        return true;
    }

    void handleRead()
    {
        unsigned long currentTime = millis();
        if (currentTime - data.lastReadTime >= readInterval)
        {
            float newTemp = dht.readTemperature();
            float newHumidity = dht.readHumidity();

            if (!isnan(newTemp) && !isnan(newHumidity))
            {
                data.temperature = newTemp;
                data.humidity = newHumidity;
                data.isValid = true;
                data.lastReadTime = currentTime;
            }
            else
            {
                data.isValid = false;
            }
        }
    }

    DHTData getData() const
    {
        return data;
    }

    bool isTemperatureInRange() const
    {
        return data.isValid &&
            (data.temperature >= tempMin && data.temperature <= tempMax);
    }

    bool isHumidityInRange() const
    {
        return data.isValid &&
            (data.humidity >= humidityMin && data.humidity <= humidityMax);
    }

    bool isInDanger() const
    {
        return data.isValid &&
            (!isTemperatureInRange() || !isHumidityInRange());
    }

    void setThresholds(float tMin, float tMax, float hMin, float hMax)
    {
        tempMin = tMin;
        tempMax = tMax;
        humidityMin = hMin;
        humidityMax = hMax;
    }

    void log() const
    {
        if (data.isValid)
        {
            Serial.printf("DHT11 - Nhiệt độ: %.0f°C, Độ ẩm: %.0f%%",
                data.temperature, data.humidity);

            if (isInDanger())
            {
                Serial.print(" [CẢNH BÁO]");
                if (!isTemperatureInRange())
                {
                    Serial.printf(" - Nhiệt độ ngoài phạm vi (%.0f-%.0f°C)",
                        tempMin, tempMax);
                }
                if (!isHumidityInRange())
                {
                    Serial.printf(" - Độ ẩm ngoài phạm vi (%.0f-%.0f%%)",
                        humidityMin, humidityMax);
                }
            }
            Serial.println();
        }
        else
        {
            Serial.println("DHT11 - Lỗi đọc dữ liệu");
        }
    }
};

#endif