#ifndef WEIGHT_TRACKING_H
#define WEIGHT_TRACKING_H

#include "HX711.h"

class WeightTracking {
private:
    HX711 scale;
    float lastWeight;
    float changeThreshold;

public:
    WeightTracking(uint8_t doutPin, uint8_t sckPin, float threshold = 50.0)
        : lastWeight(0), changeThreshold(threshold) 
    {
        scale.set_pin(doutPin, sckPin);
        scale.begin();
    }

    bool begin() {        
        Serial.println("Đang tare cân...");
        scale.set_scale();
        scale.tare(10); 

        // Đặt calibration factor (cần được tính toán từ calibration)
        scale.set_scale(5000.0);
        
        // Đọc giá trị ban đầu
        delay(1000); // Đợi ổn định
        lastWeight = scale.get_units(5); // Đọc 5 lần và lấy trung bình
        
        Serial.print("Trọng lượng ban đầu: ");
        Serial.println(lastWeight);
        
        return true;
    }

    bool checkWeightChange() {
        float currentWeight = scale.get_units();
        Serial.println(currentWeight);
        if (fabs(currentWeight - lastWeight) >= changeThreshold) {
            lastWeight = currentWeight;
            return true;
        }
        return false;
    }

    float getCurrentWeight() {
        return lastWeight;
    }
};

#endif
