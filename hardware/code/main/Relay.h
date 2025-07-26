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

    const int cycleDuration = 1000; // 1 giây
    const int dutyCycle[4] = {0, 250, 500, 750}; // thời gian bật (ms) cho từng mode

public:
    RelayController(int pin)
    {
        relayPin = pin;
        currentMode = FAN_OFF;
        lastUpdate = 0;
        cycleStart = 0;
    }

    bool begin()
    {
        pinMode(relayPin, OUTPUT);
        digitalWrite(relayPin, LOW);
        return true;
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

    void nextMode()
    {
        FanMode next = (FanMode)((currentMode + 1) % 4);
        setMode(next);
    }

    const char* getModeString(FanMode mode) const
    {
        switch (mode)
        {
        case FAN_OFF: return "TẮT";
        case FAN_LOW: return "NHẸ";
        case FAN_MEDIUM: return "TRUNG BÌNH";
        case FAN_HIGH: return "MẠNH";
        default: return "KHÔNG XÁC ĐỊNH";
        }
    }


    void log() const
    {
        Serial.print("Chế độ quạt hiện tại: ");
        Serial.println(getModeString(currentMode));
    }
};

#endif
