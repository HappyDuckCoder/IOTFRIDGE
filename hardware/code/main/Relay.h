#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

enum FanMode
{
    FAN_OFF = 0,    // Tắt quạt
    FAN_LOW = 1,    // Quạt chậm
    FAN_MEDIUM = 2, // Quạt trung bình
    FAN_HIGH = 3    // Quạt nhanh
};

struct RelayData
{
    FanMode currentMode;
    bool isActive;
    unsigned long lastModeChange;

    RelayData()
    {
        currentMode = FAN_OFF;
        isActive = false;
        lastModeChange = 0;
    }
};

class RelayController
{
private:
    int relayPin;
    int pwmPin;     // Pin PWM để điều khiển tốc độ quạt
    int pwmChannel; // PWM channel
    RelayData data;

    // PWM settings
    const int pwmFreq = 1000;
    const int pwmResolution = 8;

    // Tốc độ quạt cho các chế độ (0-255)
    const int fanSpeeds[4] = {0, 85, 170, 255}; // OFF, LOW, MEDIUM, HIGH

public:
    RelayController(int relay, int pwm, int channel = 0)
    {
        relayPin = relay;
        pwmPin = pwm;
        pwmChannel = channel;
    }

    bool begin()
    {
        pinMode(relayPin, OUTPUT);
        digitalWrite(relayPin, LOW);

        // Thiết lập PWM
        ledcSetup(pwmChannel, pwmFreq, pwmResolution);
        ledcAttachPin(pwmPin, pwmChannel);
        ledcWrite(pwmChannel, 0);

        Serial.println("Relay controller khởi tạo thành công");
        return true;
    }

    void setMode(FanMode mode)
    {
        if (data.currentMode != mode)
        {
            data.currentMode = mode;
            data.lastModeChange = millis();
            updateFanSpeed();

            Serial.printf("Chế độ quạt: %s\n", getModeString(mode));
        }
    }

    void nextMode()
    {
        FanMode nextMode = (FanMode)((data.currentMode + 1) % 4);
        setMode(nextMode);
    }

    void updateFanSpeed()
    {
        int speed = fanSpeeds[data.currentMode];

        if (data.currentMode == FAN_OFF)
        {
            digitalWrite(relayPin, LOW);
            data.isActive = false;
        }
        else
        {
            digitalWrite(relayPin, HIGH);
            data.isActive = true;
        }

        ledcWrite(pwmChannel, speed);
    }

    const char *getModeString(FanMode mode) const
    {
        switch (mode)
        {
        case FAN_OFF:
            return "TẮT";
        case FAN_LOW:
            return "CHẬM";
        case FAN_MEDIUM:
            return "TRUNG BÌNH";
        case FAN_HIGH:
            return "NHANH";
        default:
            return "KHÔNG XÁC ĐỊNH";
        }
    }

    RelayData getData() const
    {
        return data;
    }

    void log() const
    {
        Serial.printf("Quạt: %s", getModeString(data.currentMode));
        if (data.isActive)
        {
            Serial.print(" (HOẠT ĐỘNG)");
        }
        else
        {
            Serial.print(" (TẮT)");
        }
        Serial.println();
    }
};

#endif