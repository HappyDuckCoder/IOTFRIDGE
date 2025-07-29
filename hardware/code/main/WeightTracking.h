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
        : lastWeight(0), changeThreshold(threshold) {
        scale.begin(doutPin, sckPin);
    }

    void begin() {
        scale.set_scale();      // Cần calibration cụ thể
        scale.tare();
        lastWeight = scale.get_units();
    }

    bool checkWeightChange() {
        float currentWeight = scale.get_units();
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
