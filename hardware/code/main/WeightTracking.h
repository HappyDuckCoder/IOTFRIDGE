#ifndef WEIGHT_TRACKING_H
#define WEIGHT_TRACKING_H

#include "HX711.h"

class WeightTracking {
private:
    HX711 scale;
    float lastWeight;
    float changeThreshold;
    float calibrationFactor;
    bool isInitialized;
    
    // Caching để tránh đọc HX711 liên tục
    float cachedWeight;
    unsigned long lastReadTime;
    static const unsigned long READ_INTERVAL_MS = 100; // Đọc tối đa 10 lần/giây
    
    // Filter để làm mịn giá trị đọc
    static const uint8_t FILTER_SIZE = 5;
    float readings[FILTER_SIZE];
    uint8_t readIndex;
    bool filterFilled;
    
    // Helper method để đọc giá trị có filter
    float getFilteredReading() {
        float rawReading = scale.get_units();
        
        // Thêm vào buffer
        readings[readIndex] = rawReading;
        readIndex = (readIndex + 1) % FILTER_SIZE;
        
        if (!filterFilled && readIndex == 0) {
            filterFilled = true;
        }
        
        // Tính trung bình
        float sum = 0;
        uint8_t count = filterFilled ? FILTER_SIZE : readIndex;
        
        for (uint8_t i = 0; i < count; i++) {
            sum += readings[i];
        }
        
        return count > 0 ? sum / count : rawReading;
    }
    
    // Helper method để kiểm tra và cập nhật cache
    void updateCacheIfNeeded() {
        unsigned long currentTime = millis();
        
        // Xử lý overflow của millis()
        bool shouldUpdate = false;
        if (currentTime >= lastReadTime) {
            shouldUpdate = (currentTime - lastReadTime >= READ_INTERVAL_MS);
        } else {
            // Overflow case
            shouldUpdate = true;
        }
        
        if (shouldUpdate) {
            cachedWeight = getFilteredReading();
            lastReadTime = currentTime;
        }
    }

    // Reset filter buffer
    void resetFilter() {
        for (uint8_t i = 0; i < FILTER_SIZE; i++) {
            readings[i] = 0;
        }
        readIndex = 0;
        filterFilled = false;
    }

public:
    WeightTracking(uint8_t doutPin, uint8_t sckPin, float threshold = 50.0)
        : lastWeight(0), 
          changeThreshold(threshold),
          calibrationFactor(5000.0),
          isInitialized(false),
          cachedWeight(0),
          lastReadTime(0),
          readIndex(0),
          filterFilled(false)
    {
        scale.set_pin(doutPin, sckPin);
        scale.begin();
        resetFilter();
    }

    bool begin() {
        if (isInitialized) {
            return true; // Đã khởi tạo rồi
        }
        
        Serial.println("Loadcell taring");
        
        // Kiểm tra HX711 có sẵn sàng không
        if (!scale.is_ready()) {
            Serial.println("HX711 failed");
            return false;
        }
        
        scale.set_scale();
        scale.tare(10); 

        // Đặt calibration factor
        scale.set_scale(calibrationFactor);
        
        // Đợi ổn định và fill filter buffer
        Serial.println("Loadcell readying");
        delay(1000);
        
        // Fill filter buffer với các giá trị ban đầu
        for (uint8_t i = 0; i < FILTER_SIZE; i++) {
            float reading = scale.get_units();
            readings[i] = reading;
            delay(100); // Đợi giữa các lần đọc
        }
        filterFilled = true;
        
        // Đọc giá trị ban đầu đã được filter
        lastWeight = getFilteredReading();
        cachedWeight = lastWeight;
        lastReadTime = millis();
        
        Serial.print("Current Weight: ");
        Serial.print(lastWeight);
        Serial.println(" g");
        
        isInitialized = true;
        return true;
    }

    bool checkWeightChange() {
        if (!isInitialized) {
            Serial.println("Begin() failed");
            return false;
        }
        
        // Kiểm tra HX711 có sẵn sàng không
        if (!scale.is_ready()) {
            Serial.println("HX711 Failed");
            return false;
        }
        
        updateCacheIfNeeded();
        float currentWeight = cachedWeight;
        
        Serial.print("Current: ");
        Serial.print(currentWeight);
        Serial.print("g, Last: ");
        Serial.print(lastWeight);
        Serial.print("g, Diff: ");
        Serial.print(fabs(currentWeight - lastWeight));
        Serial.println("g");
        
        if (fabs(currentWeight - lastWeight) >= changeThreshold) {
            lastWeight = currentWeight;
            return true;
        }
        return false;
    }

    float getCurrentWeight() {
        if (!isInitialized) {
            return 0;
        }
        
        updateCacheIfNeeded();
        return cachedWeight;
    }
    
    // Utility methods mới
    bool isReady() {
        return isInitialized && scale.is_ready();
    }
    
    void setCalibrationFactor(float factor) {
        calibrationFactor = factor;
        if (isInitialized) {
            scale.set_scale(calibrationFactor);
        }
    }
    
    float getCalibrationFactor() const {
        return calibrationFactor;
    }
    
    void setChangeThreshold(float threshold) {
        changeThreshold = threshold;
    }
    
    float getChangeThreshold() const {
        return changeThreshold;
    }
    
    void recalibrate() {
        if (!isInitialized) {
            return;
        }
        
        Serial.println("Đang recalibrate...");
        scale.tare(10);
        resetFilter();
        
        // Fill lại filter buffer
        delay(500);
        for (uint8_t i = 0; i < FILTER_SIZE; i++) {
            readings[i] = scale.get_units();
            delay(100);
        }
        filterFilled = true;
        
        lastWeight = getFilteredReading();
        cachedWeight = lastWeight;
        Serial.println("Recalibrate ");
    }
    
    // Đọc raw value để debug
    long getRawValue() {
        return scale.read();
    }
    
    // Reset về trạng thái ban đầu
    void reset() {
        isInitialized = false;
        lastWeight = 0;
        cachedWeight = 0;
        lastReadTime = 0;
        resetFilter();
    }
};

#endif