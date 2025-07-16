#ifndef GasSensor_h
#define GasSensor_h

#include <Arduino.h>
#include "constant.h"

class GasSensor
{
public:
    int adcMQ2 = 0;
    int adcMQ135 = 0;
    float ppmCH4 = 0.0;
    float ppmNH3 = 0.0;
    float rsMQ2 = 0.0;
    float rsMQ135 = 0.0;
    float ratioMQ2 = 0.0;
    float ratioMQ135 = 0.0;
    float RoMQ2 = 1.0;
    float RoMQ135 = 1.0;
    bool calibrated = false;

    const float RL = 10000.0;
    const float Vcc = 3.3;
    const float MQ2_A = -0.38, MQ2_B = 1.48;
    const float MQ135_A = -0.45, MQ135_B = 2.95;
    const float CH4_THRESHOLD = 3000.0;
    const float NH3_THRESHOLD = 50.0;

    void calibrate()
    {
        long sumMQ2 = 0, sumMQ135 = 0;
        int samples = 100;
        Serial.println("Hiệu chuẩn MQ-2 và MQ-135...");

        for (int i = 0; i < samples; i++)
        {
            sumMQ2 += calculateRs(analogRead(MQ2_PIN));
            sumMQ135 += calculateRs(analogRead(MQ135_PIN));
            delay(100);
        }

        RoMQ2 = (sumMQ2 / (float)samples) / 4.4;
        RoMQ135 = (sumMQ135 / (float)samples) / 3.7;

        Serial.printf("Ro MQ-2 = %.2f | Ro MQ-135 = %.2f\n", RoMQ2, RoMQ135);
        calibrated = true;
    }

    void handleRead()
    {
        if (!calibrated)
            return;

        adcMQ2 = analogRead(MQ2_PIN);
        adcMQ135 = analogRead(MQ135_PIN);

        rsMQ2 = calculateRs(adcMQ2);
        rsMQ135 = calculateRs(adcMQ135);

        ratioMQ2 = rsMQ2 / RoMQ2;
        ratioMQ135 = rsMQ135 / RoMQ135;

        ppmCH4 = calculatePPM(ratioMQ2, MQ2_A, MQ2_B);
        ppmNH3 = calculatePPM(ratioMQ135, MQ135_A, MQ135_B);
    }

    bool isInDanger()
    {
        return (ppmCH4 > CH4_THRESHOLD || ppmNH3 > NH3_THRESHOLD);
    }

    void log()
    {
        Serial.printf("CH4: %.1f ppm\tNH3: %.1f ppm\t", ppmCH4, ppmNH3);
        if (isInDanger())
        {
            Serial.println("NGUY HIỂM!");
        }
        else
        {
            Serial.println("An toàn.");
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