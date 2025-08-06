#ifndef BUTTON_H
#define BUTTON_H

class Button
{
private:
    int pin;
    bool pressed;
    bool lastState;
    unsigned long lastPressTime;
    unsigned long debounceDelay;
    unsigned long pressStartTime;

public:
    Button(int buttonPin, unsigned long debounce = 200)
    {
        pin = buttonPin;
        pressed = false;
        lastState = false;
        lastPressTime = 0;
        debounceDelay = debounce;
        pressStartTime = 0;
    }

    bool begin()
    {
        pinMode(pin, INPUT);

        // Đọc tín hiệu thử, nếu đọc được -> kết nối thành công
        int state = digitalRead(pin);
        return (state == LOW || state == HIGH);
    }

    bool isPressed()
    {
        bool currentState = digitalRead(pin);
        bool result = false;

        if (currentState == LOW && lastState == HIGH &&
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

    bool isHeldtSecond(int sec)
    {
        static bool alreadySwitched = false;

        unsigned long now = millis();
        unsigned long ms = sec * 1000;

        if (digitalRead(pin) == HIGH)
        {
            if (pressStartTime == 0)
                pressStartTime = now;

            if (!alreadySwitched && (now - pressStartTime >= ms))
            {
                alreadySwitched = true;
                return true;
            }
        }
        else
        {
            pressStartTime = 0;
            alreadySwitched = false; // reset khi thả nút
        }

        return false;
    }

};

#endif