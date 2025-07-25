#ifndef HANDLE_DELAY_H
#define HANDLE_DELAY_H

class HandleDelay
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

    void reset()
    {
        lastMillis = millis();
    }

    void setInterval(unsigned long ms)
    {
        interval = ms;
    }
};

#endif