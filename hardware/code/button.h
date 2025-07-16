#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button
{
private:
    int pin;
    bool pressed;
    bool lastState;
    unsigned long lastPressTime;
    unsigned long debounceDelay;

public:
    Button(int buttonPin, unsigned long debounce = 200)
    {
        pin = buttonPin;
        pressed = false;
        lastState = false;
        lastPressTime = 0;
        debounceDelay = debounce;
        pinMode(pin, INPUT);
    }

    bool isPressed()
    {
        bool currentState = digitalRead(pin);
        bool result = false;

        // Kiểm tra trạng thái thay đổi từ LOW sang HIGH
        if (currentState == HIGH && !lastState &&
            (millis() - lastPressTime > debounceDelay))
        {
            result = true;
            lastPressTime = millis();
        }

        lastState = currentState;
        return result;
    }

    bool isHeld()
    {
        return digitalRead(pin) == HIGH;
    }
};

#endif