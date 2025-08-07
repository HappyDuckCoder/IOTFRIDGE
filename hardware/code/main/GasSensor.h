#ifndef GASSENSOR_H
#define GASSENSOR_H

#include <math.h>

// Base class cho tất cả gas sensor
class GasSensor
{
protected:
    int pin;
    float Rs = 0.0;
    float ratio = 0.0;
    float Ro = 1.0;
    float ppm = 0.0;

    bool calibrated = false;
    bool dataValid = false;

    const float RL = 10000.0; // điện trở tải (ohm)
    const float Vcc = 3.3;    // điện áp cấp cho cảm biến

    // Hằng số đường cong cảm biến (sẽ được override trong class con)
    float sensorA = 0.0;
    float sensorB = 0.0;
    float roRatio = 1.0; // Tỉ lệ Rs/Ro trong không khí sạch

    String sensorName;
    String gasType;
    float threshold = 0.0;

public:
    GasSensor(int sensorPin) : pin(sensorPin) {}

    virtual bool begin()
    {
        pinMode(pin, INPUT);
        return true;
    }

    virtual void calibrate(int samples = 100)
    {
        long sum = 0;

        for (int i = 0; i < samples; i++)
        {
            sum += calculateRs(analogRead(pin));
            delay(100);
        }

        Ro = (sum / (float)samples) / roRatio;
        calibrated = true;
    }

    virtual void handleRead()
    {
        {
            if (!calibrated)
                return;

            int adc = analogRead(pin);

            Rs = calculateRs(adc);
            if (Rs <= 0)
            {
                dataValid = false;
                ppm = 0;
                return;
            }

            ratio = Rs / Ro;
            if (ratio <= 0)
            {
                dataValid = false;
                ppm = 0;
                return;
            }

            ppm = calculatePPM(ratio, sensorA, sensorB);
            dataValid = !isnan(ppm) && ppm >= 0;
        }
    }

    float getPPM() const { return ppm; }
    bool isDataValid() const { return dataValid && calibrated; }
    bool isInDanger() const { return isDataValid() && ppm > threshold; }
    String getSensorName() const { return sensorName; }
    String getGasType() const { return gasType; }
    float getThreshold() const { return threshold; }
    void setThreshold(float newThreshold) { threshold = newThreshold; }

    virtual void log() const
    {
        if (!isDataValid())
        {
            Serial.println(sensorName + " - Dữ liệu lỗi");
            return;
        }

        Serial.printf("%s - %s: %.1f ppm ", sensorName.c_str(), gasType.c_str(), ppm);
        Serial.println("");

        if (isInDanger())
        {
            Serial.print("Có khí độc thoát ra, cảm biến từ: ");
            Serial.print(sensorName);
        }
    }

protected:
    float calculateRs(int adc)
    {
        float Vout = adc * Vcc / 4095.0;
        if (Vout < 0.01)
            return -1.0;
        return RL * (Vcc - Vout) / Vout;
    }

    float calculatePPM(float ratio, float a, float b)
    {
        if (ratio <= 0)
            return 0;
        return pow(10, (a * log10(ratio) + b));
    }
};

// MQ-2 Sensor cho CH4 (Methane)
class MQ2Sensor : public GasSensor
{
public:
    MQ2Sensor(int pin) : GasSensor(pin)
    {
        sensorName = "MQ-2";
        gasType = "CH4";
        sensorA = -0.38;
        sensorB = 1.48;
        roRatio = 4.4;      // Rs/Ro trong không khí sạch
        threshold = 3000.0; // ppm
    }
};

// MQ-135 Sensor cho NH3 (Ammonia)
class MQ135Sensor : public GasSensor
{
public:
    MQ135Sensor(int pin) : GasSensor(pin)
    {
        sensorName = "MQ-135";
        gasType = "NH3";
        sensorA = -0.45;
        sensorB = 2.95;
        roRatio = 3.7;    // Rs/Ro trong không khí sạch
        threshold = 50.0; // ppm
    }
};

// Struct để chứa dữ liệu tổng hợp
struct GasSystemData
{
    float ppmCH4;
    float ppmNH3;
    bool ch4Valid;
    bool nh3Valid;
    bool systemDanger;

    GasSystemData()
    {
        ppmCH4 = 0.0;
        ppmNH3 = 0.0;
        ch4Valid = false;
        nh3Valid = false;
        systemDanger = false;
    }
};

// System quản lý các gas sensor
class GasSensorSystem
{
private:
    MQ2Sensor mq2Sensor;
    MQ135Sensor mq135Sensor;
    GasSystemData systemData;

public:
    GasSensorSystem(int mq2Pin, int mq135Pin) : mq2Sensor(mq2Pin), mq135Sensor(mq135Pin) {}

    void calibrate(int samples = 100)
    {
        mq2Sensor.calibrate(samples);
        mq135Sensor.calibrate(samples);
    }

    bool begin()
    {
        bool mq2Ok = mq2Sensor.begin();
        bool mq135Ok = mq135Sensor.begin();

        calibrate(100); // calibrate trong begin luôn

        if (mq2Ok && mq135Ok)
        {
            return true;
        }
        else
            return false;
    }

    void handleRead()
    {
        mq2Sensor.handleRead();
        mq135Sensor.handleRead();
        updateSystemData();
    }

    GasSystemData getSystemData() const
    {
        return systemData;
    }

    MQ2Sensor &getMQ2Sensor() { return mq2Sensor; }
    MQ135Sensor &getMQ135Sensor() { return mq135Sensor; }

    bool isSystemInDanger() const
    {
        return systemData.systemDanger;
    }

    void setThresholds(float ch4Threshold, float nh3Threshold)
    {
        mq2Sensor.setThreshold(ch4Threshold);
        mq135Sensor.setThreshold(nh3Threshold);
    }

    void log() const
    {
        mq2Sensor.log();
        mq135Sensor.log();

        if (isSystemInDanger())
            Serial.println(" - Có khí bị hư");
    }

private:
    void updateSystemData()
    {
        systemData.ppmCH4 = mq2Sensor.getPPM();
        systemData.ppmNH3 = mq135Sensor.getPPM();
        systemData.ch4Valid = mq2Sensor.isDataValid();
        systemData.nh3Valid = mq135Sensor.isDataValid();
        systemData.systemDanger = mq2Sensor.isInDanger() || mq135Sensor.isInDanger();
    }
};

#endif