
void print()
{
    String t = "Hello World";
    Serial.printf("%s", t);
    Serial.println();
}

class HandleDelay()
{
private:
    unsigned long lastMillis;
    unsigned long interval;

public:
    HandleDelay(unsigned long ms)
    {
        interval = ms;
        lastMillis = millis();
    }

    bool isDue()
    {
        unsigned long currentMillis = millis();
        if (currentMillis - lastMillis >= interval)
        {
            lastMillis = currentMillis;
            return true;
        }
        return false;
    }
};

class Button
{
private:
    int pin;
    unsigned long lassPressTime;
    unsigned long debounce;
    bool lastState = false;

public:
    Button() : pin(0), lassPressTime(0), debounce(0), lastState(false) {}
    Button(int pin, unsigned int debounce) : pin(pin), lassPressTime(0), debounce(debounce), lastState(false) {}

    bool begin()
    {
        pinMode(pin, INPUT);
        int state = digitalRead(pin);
        return (state == LOW || state == HIGH);
    }

    bool isPress()
    {
        int currentState = digitalRead(pin);
        res = false;

        if (currentState == LOW && lastState == HIGHT && (millis() - lassPressTime > debounce))
        {
            res = true;
            lassPressTime = millis();
        }

        lastState = currentState;
        return res;
    }
};