#ifndef GASSENSOR_H
#define GASSENSOR_H

#include <math.h>
#include "constant.h"

struct GasData
{
    float ppmCH4;
    float ppmNH3;
    bool isValid;
    unsigned long lastReadTime;

    GasData()
    {
        ppmCH4 = 0.0;
        ppmNH3 = 0.0;
        isValid = false;
        lastReadTime = 0;
    }
};

class GasSensor
{
private:
    const int MQ2_PIN;
    const int MQ135_PIN;

    GasData data;

    int adcMQ2 = 0;
    int adcMQ135 = 0;

    float rsMQ2 = 0.0;
    float rsMQ135 = 0.0;

    float ratioMQ2 = 0.0;
    float ratioMQ135 = 0.0;

    float RoMQ2 = 1.0;
    float RoMQ135 = 1.0;

    unsigned long readInterval;
    bool calibrated = false;

    const float RL = 10000.0; // điện trở tải (ohm)
    const float Vcc = 3.3;    // điện áp cấp cho cảm biến

    // Hằng số đường cong cảm biến
    const float MQ2_A = -0.38, MQ2_B = 1.48;
    const float MQ135_A = -0.45, MQ135_B = 2.95;

    // Ngưỡng cảnh báo (có thể cho chỉnh sửa qua setThreshold sau này)
    const float CH4_THRESHOLD = 3000.0;
    const float NH3_THRESHOLD = 50.0;

public:
    GasSensor(MQ2_PIN, MQ135_PIN, unsigned long interval = 5000)
        : readInterval(interval)
    {
        this->MQ2_PIN = MQ2_PIN;
        this->MQ135_PIN = MQ135_PIN;
    }

    bool begin()
    {
        pinMode(MQ2_PIN, INPUT);
        pinMode(MQ135_PIN, INPUT);
        Serial.println("Khởi động GasSensor...");
        return true;
    }

    void calibrate(int samples = 100)
    {
        long sumMQ2 = 0, sumMQ135 = 0;

        Serial.println("Đang hiệu chuẩn MQ-2 và MQ-135...");

        for (int i = 0; i < samples; i++)
        {
            sumMQ2 += calculateRs(analogRead(MQ2_PIN));
            sumMQ135 += calculateRs(analogRead(MQ135_PIN));
            delay(100);
        }

        RoMQ2 = (sumMQ2 / (float)samples) / 4.4;
        RoMQ135 = (sumMQ135 / (float)samples) / 3.7;

        calibrated = true;

        Serial.printf("Hiệu chuẩn xong: Ro MQ-2 = %.2f | Ro MQ-135 = %.2f\n", RoMQ2, RoMQ135);
    }

    void handleRead()
    {
        if (!calibrated)
            return;

        unsigned long currentTime = millis();
        if (currentTime - data.lastReadTime >= readInterval)
        {
            adcMQ2 = analogRead(MQ2_PIN);
            adcMQ135 = analogRead(MQ135_PIN);

            rsMQ2 = calculateRs(adcMQ2);
            rsMQ135 = calculateRs(adcMQ135);

            ratioMQ2 = rsMQ2 / RoMQ2;
            ratioMQ135 = rsMQ135 / RoMQ135;

            data.ppmCH4 = calculatePPM(ratioMQ2, MQ2_A, MQ2_B);
            data.ppmNH3 = calculatePPM(ratioMQ135, MQ135_A, MQ135_B);
            data.lastReadTime = currentTime;
            data.isValid = true;
        }
    }

    GasData getData() const
    {
        return data;
    }

    bool isInDanger() const
    {
        return data.isValid &&
               (data.ppmCH4 > CH4_THRESHOLD || data.ppmNH3 > NH3_THRESHOLD);
    }

    void log() const
    {
        if (!data.isValid)
        {
            Serial.println("GasSensor - Dữ liệu không hợp lệ.");
            return;
        }

        Serial.printf("GasSensor - CH4: %.1f ppm | NH3: %.1f ppm", data.ppmCH4, data.ppmNH3);

        if (isInDanger())
        {
            Serial.println("Nguy hiểm");
        }
        else
        {
            Serial.println("An toàn");
        }
    }

private:
    float calculateRs(int adc)
    {
        float Vout = adc * Vcc / 4095.0;
        return RL * (Vcc - Vout) / Vout;
    }

    float calculatePPM(float ratio, float a, float b)
    {
        return pow(10, (a * log10(ratio) + b));
    }
};

#endif
