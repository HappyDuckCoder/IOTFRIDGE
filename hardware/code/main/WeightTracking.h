#ifndef WEIGHT_TRACKING_H
#define WEIGHT_TRACKING_H

#include "HX711.h"

class WeightTracking
{
private:
    HX711 scale;
    float lastWeight;
    float changeThreshold;

    // hàm nội bộ để tính trọng lượng trung bình trong 1s
    float getAverageWeight1s()
    {
        unsigned long startTime = millis();
        float sum = 0.0;
        int count = 0;

        while (millis() - startTime < 1000) // đo trong 1 giây
        {
            sum += scale.get_units();
            count++;
            delay(50); // tránh đọc quá nhanh, ~20 mẫu/giây
        }

        if (count == 0) return 0.0;
        return sum / count;
    }

public:
    WeightTracking(uint8_t doutPin, uint8_t sckPin, float threshold = 50.0)
        : lastWeight(0), changeThreshold(threshold)
    {
        scale.set_pin(doutPin, sckPin);
        scale.begin();
    }

    bool begin()
    {
        Serial.println("Đang tare cân...");
        scale.set_scale();
        scale.tare(10);

        // calibration factor - cần tự tính toán
        scale.set_scale(5000.0);

        // Đọc giá trị ban đầu
        delay(1000);                     
        lastWeight = getAverageWeight1s();

        Serial.print("Trọng lượng ban đầu: ");
        Serial.println(lastWeight);

        return true;
    }

    bool checkWeightChange()
{
    float currentWeight = getAverageWeight1s(); // chỉ đo một lần
    Serial.println(currentWeight); // in ra luôn giá trị vừa đo

    if (fabs(currentWeight - lastWeight) >= changeThreshold)
    {
        lastWeight = currentWeight;
        return true;
    }
    return false;
}


    float getCurrentWeight()
    {
        return lastWeight;
    }
};

#endif