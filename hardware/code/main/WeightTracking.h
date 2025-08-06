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
        if (!scale.is_ready()) {
            Serial.println("HX711 không sẵn sàng!");
            return false;
        }
        
        Serial.println("Đang tare cân...");
        scale.set_scale();
        scale.tare(10); // Tare với 10 lần đọc để độ chính xác cao hơn
        
        // Đặt calibration factor (cần được tính toán từ calibration)
        scale.set_scale(1000.0);
        
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
