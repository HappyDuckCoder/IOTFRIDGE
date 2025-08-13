#ifndef DOOR_TRACKING_H
#define DOOR_TRACKING_H

#include <Arduino.h>

enum DoorState {
    DOOR_OPEN = 0,
    DOOR_CLOSED = 1,
    DOOR_NOT_FIRM = 2
};

class DoorTracking {
private:
    unsigned long openStartTime = 0;
    const unsigned long alertThreshold;
    const unsigned long justClosedThreshold; 

    DoorState currentState = DOOR_CLOSED;
    DoorState lastState = DOOR_CLOSED;

    bool alertSent = false;
    bool justClosedFlag = false;

    // Helper method để tránh lặp lại code
    inline void resetAlertState() {
        alertSent = false;
    }

    // Helper method để kiểm tra transition
    inline bool isTransition(DoorState from, DoorState to) const {
        return (lastState == from && currentState == to);
    }

public:
    // alertThreshold mặc định là 30 giây, justClosedThreshold là 20 giây
    DoorTracking(unsigned long thresholdMs = 30000, unsigned long justClosedThreshold = 20000)
        : alertThreshold(thresholdMs), justClosedThreshold(justClosedThreshold) {}

    void setCurrentState(DoorState state) {
        currentState = state;

        // Xử lý transition từ CLOSED sang OPEN
        if (isTransition(DOOR_CLOSED, DOOR_OPEN)) {
            openStartTime = millis();
            resetAlertState();
        }
        // Xử lý transition từ OPEN sang CLOSED
        else if (isTransition(DOOR_OPEN, DOOR_CLOSED)) {
            const unsigned long duration = millis() - openStartTime;
            justClosedFlag = (duration < justClosedThreshold);
            openStartTime = 0;
            resetAlertState();
        }

        lastState = currentState;
    }

    bool isAlertNeeded() {
        // Chỉ check alert khi cửa đang mở và chưa gửi alert
        if (currentState == DOOR_OPEN && !alertSent) {
            const unsigned long currentTime = millis();
            
            // Xử lý overflow của millis() (xảy ra sau ~50 ngày)
            const unsigned long elapsed = (currentTime >= openStartTime) 
                ? (currentTime - openStartTime)
                : (ULONG_MAX - openStartTime + currentTime + 1);
                
            if (elapsed >= alertThreshold) {
                alertSent = true;
                return true;
            }
        }
        return false;
    }

    bool isDoorJustClosed() {
        if (justClosedFlag) {
            justClosedFlag = false; // Reset flag sau khi đọc
            return true;
        }
        return false;
    }

    // Thêm các utility methods hữu ích
    DoorState getCurrentState() const {
        return currentState;
    }

    DoorState getLastState() const {
        return lastState;
    }

    unsigned long getOpenDuration() const {
        if (currentState == DOOR_OPEN && openStartTime > 0) {
            const unsigned long currentTime = millis();
            return (currentTime >= openStartTime) 
                ? (currentTime - openStartTime)
                : (ULONG_MAX - openStartTime + currentTime + 1);
        }
        return 0;
    }

    bool isAlertSent() const {
        return alertSent;
    }

    void log() const {
        Serial.print("Current State: ");
        Serial.print(currentState);
        Serial.print(" | Last State: ");
        Serial.print(lastState);
        Serial.print(" | Alert Sent: ");
        Serial.print(alertSent);
        if (currentState == DOOR_OPEN) {
            Serial.print(" | Open Duration: ");
            Serial.print(getOpenDuration());
            Serial.print("ms");
        }
        Serial.println();
    }
};

#endif