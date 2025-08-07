#ifndef DOOR_TRACKING_H
#define DOOR_TRACKING_H

#include <Arduino.h>

enum DoorState
{
    DOOR_OPEN = 0,
    DOOR_CLOSED = 1,
    DOOR_NOT_FIRM = 2
};

class DoorTracking
{
private:
    unsigned long openStartTime = 0;
    unsigned long alertThreshold;
    unsigned long justClosedThreshold;

    DoorState currentState = DOOR_CLOSED;
    DoorState lastState = DOOR_CLOSED;

    bool alertSent = false;
    bool justClosedFlag = false;

public:
    // alertThreshold mặc định là 5 phút
    DoorTracking(unsigned long thresholdMs = 30000, unsigned long justClosedThreshold = 20000)
        : alertThreshold(thresholdMs), justClosedThreshold(justClosedThreshold) {}

    void setCurrentState(DoorState state)
    {
        currentState = state;

        // Nếu cửa vừa mở thì bắt đầu đếm thời gian
        if (lastState == DOOR_CLOSED && currentState == DOOR_OPEN)
        {
            openStartTime = millis();
            alertSent = false;
        }

        // Nếu cửa vừa đóng lại thì kiểm tra thời gian mở
        if (lastState == DOOR_OPEN && currentState == DOOR_CLOSED)
        {
            unsigned long duration = millis() - openStartTime;
            justClosedFlag = (duration < justClosedThreshold);
            openStartTime = 0;
            alertSent = false;
        }

        lastState = currentState;
    }

    bool isAlertNeeded()
    {
        if (currentState == DOOR_OPEN)
        {
            if (!alertSent && (millis() - openStartTime >= alertThreshold))
            {
                alertSent = true;
                return true;
            }
        }
        return false;
    }

    bool isDoorJustClosed()
    {
        if (justClosedFlag)
        {
            justClosedFlag = false;
            return true;
        }
        return false;
    }

    void log()
    {
        Serial.print("current state: ");
        Serial.println(currentState);
    }
};

#endif
