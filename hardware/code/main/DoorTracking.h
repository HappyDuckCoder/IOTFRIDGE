#ifndef DOOR_TRACKING_H
#define DOOR_TRACKING_H

#include <Arduino.h>

enum DoorState {
    DOOR_OPEN,
    DOOR_CLOSED,
    DOOR_NOT_FIRM
};

class DoorTracking {
private:
    uint8_t buttonPin;
    unsigned long openStartTime;
    unsigned long alertThreshold;
    DoorState lastState;

public:
    DoorTracking(uint8_t pin, unsigned long thresholdMs = 120000)
        : buttonPin(pin), alertThreshold(thresholdMs), openStartTime(0), lastState(DOOR_CLOSED) {}

    void begin() {
        pinMode(buttonPin, INPUT_PULLUP);
    }

    DoorState getState() {
        int signal = digitalRead(buttonPin);
        if (signal == HIGH) return DOOR_OPEN;
        return DOOR_CLOSED;
    }

    bool isAlertNeeded() {
        DoorState currentState = getState();
        if (currentState == DOOR_OPEN) {
            if (openStartTime == 0) openStartTime = millis();
            if (millis() - openStartTime > alertThreshold) return true;
        } else {
            openStartTime = 0;
        }
        lastState = currentState;
        return false;
    }

    bool isDoorJustClosed() {
        DoorState currentState = getState();
        bool justClosed = (lastState == DOOR_OPEN && currentState == DOOR_CLOSED);
        lastState = currentState;
        return justClosed;
    }
};

#endif
