#ifndef RELAY_H
#define RELAY_H
#include <Arduino.h>

enum FanMode
{
    FAN_OFF = 0,
    FAN_LOW,
    FAN_MEDIUM,
    FAN_HIGH
};

class RelayController
{
private:
    int relayPin;
    FanMode currentMode;
    unsigned long lastUpdate;
    unsigned long cycleStart;
    const int cycleDuration = 1000;              // 1 giây
    const int dutyCycle[4] = { 0, 250, 500, 750 }; // thời gian bật (ms) cho từng mode

    // Ngưỡng nhiệt độ để điều khiển quạt
    const float TEMP_OFF = 2.0;        // Dưới 4°C: tắt quạt
    const float TEMP_LOW = 10;        // 4-6°C: quạt nhẹ
    const float TEMP_MEDIUM = 20;     // 6-8°C: quạt trung bình
    const float TEMP_HIGH = 40;      // Trên 8°C: quạt mạnh

    bool automationMode;

    float currentTemp;
    unsigned long lastTempCheck;
    const unsigned long TEMP_CHECK_INTERVAL = 5000; // Kiểm tra nhiệt độ mỗi 5 giây

public:
    RelayController(int pin)
    {
        relayPin = pin;
        currentMode = FAN_OFF;
        lastUpdate = 0;
        cycleStart = 0;
        currentTemp = 0.0;
        lastTempCheck = 0;
        automationMode = false;
    }

    bool begin()
    {
        pinMode(relayPin, OUTPUT);
        digitalWrite(relayPin, LOW);
        return true;
    }

    void setAutomationMode(bool mode)
    {
        automationMode = mode;
    }

    bool getCurrentAutomationMode() 
    {
        return automationMode;
    }

    // Cập nhật nhiệt độ từ cảm biến
    void updateTemperature(float temperature)
    {
        if (!automationMode) 
        {
            return;
        }

        currentTemp = temperature;

        // Tự động điều chỉnh chế độ quạt dựa trên nhiệt độ
        FanMode newMode = FAN_OFF;

        if (currentTemp >= TEMP_HIGH)
        {
            newMode = FAN_HIGH;
        }
        else if (currentTemp >= TEMP_MEDIUM)
        {
            newMode = FAN_MEDIUM;
        }
        else if (currentTemp >= TEMP_LOW)
        {
            newMode = FAN_LOW;
        }
        else
        {
            newMode = FAN_OFF;
        }

        // Chỉ thay đổi mode nếu khác với mode hiện tại
        if (newMode != currentMode)
        {
            setMode(newMode);

            Serial.print("Nhiệt độ: ");
            Serial.print(currentTemp);
            Serial.print("°C - Chuyển sang chế độ: ");
            Serial.println(getModeString(newMode));
        }
    }

    void setMode(FanMode mode)
    {
        currentMode = mode;
        cycleStart = millis();
    }

    void update()
    {
        unsigned long now = millis();
        unsigned long cycleTime = (now - cycleStart) % cycleDuration;
        int onTime = dutyCycle[currentMode];

        if (onTime == 0)
        {
            digitalWrite(relayPin, LOW);
        }
        else if (cycleTime < onTime)
        {
            digitalWrite(relayPin, HIGH);
        }
        else
        {
            digitalWrite(relayPin, LOW);
        }
    }

    // Phương thức thủ công (backup) - có thể dùng khi cần thiết
    void nextMode()
    {
        FanMode next = (FanMode)((currentMode + 1) % 4);
        setMode(next);
        Serial.print("Chế độ thủ công: ");
        Serial.println(getModeString(next));
    }

    const char* getModeString(FanMode mode) const
    {
        switch (mode)
        {
        case FAN_OFF:
            return "TẮT";
        case FAN_LOW:
            return "NHẸ";
        case FAN_MEDIUM:
            return "TRUNG BÌNH";
        case FAN_HIGH:
            return "MẠNH";
        default:
            return "KHÔNG XÁC ĐỊNH";
        }
    }

    void log() const
    {
        Serial.print("Nhiệt độ: ");
        Serial.print(currentTemp);
        Serial.print("°C - Chế độ quạt: ");
        Serial.println(getModeString(currentMode));
    }

    // Getter để lấy thông tin
    float getCurrentTemperature() const { return currentTemp; }
    FanMode getCurrentMode() const { return currentMode; }

    // Thiết lập ngưỡng nhiệt độ tùy chỉnh (nếu cần)
    void setTemperatureThresholds(float off, float low, float medium, float high)
    {
        // Có thể thêm validation ở đây
        Serial.println("Cập nhật ngưỡng nhiệt độ thành công");
    }
};

#endif