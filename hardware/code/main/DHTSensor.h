#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Arduino.h>
#include <DHT.h>

struct DHTData
{
    float temperature;
    float humidity;
    bool isValid;

    DHTData() : temperature(0.0), humidity(0.0), isValid(false) {}
};

class DHTSensor
{
private:
    DHT dht;
    int pin;
    int type;
    DHTData data;

    // Ngưỡng cảnh báo
    float tempMin, tempMax;
    float humidityMin, humidityMax;

public:
    DHTSensor(int dhtPin, int dhtType)
        : dht(dhtPin, dhtType), pin(dhtPin), type(dhtType),
          tempMin(18.0), tempMax(30.0), humidityMin(40.0), humidityMax(70.0) {}

    bool begin()
    {
        dht.begin();
        return true;
    }

    void handleRead()
    {
        float newTemp = dht.readTemperature();
        float newHumidity = dht.readHumidity();

        if (!isnan(newTemp) && !isnan(newHumidity))
        {
            data.temperature = newTemp;
            data.humidity = newHumidity;
            data.isValid = true;
            data.lastReadTime = millis();
        }
        else
        {
            data.isValid = false;
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