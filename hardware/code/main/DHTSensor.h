#ifndef HX711_H
#define HX711_H

#include <Arduino.h>

struct HX711Data
{
    float weight;
    long rawValue;
    bool isValid;

    HX711Data() : weight(0.0), rawValue(0), isValid(false) {}
};

class HX711Sensor
{
private:
    int PD_SCK; // Pin Clock
    int DOUT;   // Pin Data
    long OFFSET; // Giá trị offset (tare)
    float SCALE; // Hệ số scale
    HX711Data data;

    // Ngưỡng cảnh báo
    float weightMin, weightMax;

public:
    HX711Sensor(int sck, int dout)
        : PD_SCK(sck), DOUT(dout), OFFSET(0), SCALE(1.0),
          weightMin(0.0), weightMax(1000.0) {}

    bool begin()
    {
        pinMode(PD_SCK, OUTPUT);
        pinMode(DOUT, INPUT);
        return true;
    }

    // Đọc giá trị raw từ HX711 (24-bit signed)
    long read()
    {
        while (digitalRead(DOUT) == HIGH)
            ; // Chờ dữ liệu sẵn sàng

        unsigned long value = 0;
        for (int i = 0; i < 24; i++)
        {
            digitalWrite(PD_SCK, HIGH);
            delayMicroseconds(1);
            value = (value << 1) | digitalRead(DOUT);
            digitalWrite(PD_SCK, LOW);
            delayMicroseconds(1);
        }

        // Đọc thêm 1 xung clock để đặt gain mặc định 128
        digitalWrite(PD_SCK, HIGH);
        delayMicroseconds(1);
        digitalWrite(PD_SCK, LOW);
        delayMicroseconds(1);

        // Giá trị trả về là signed 24-bit
        if (value & 0x800000)
        {
            value |= ~0xFFFFFF; // Sign extend nếu bit 23 = 1
        }

        return (long)value;
    }

    // Đọc trung bình nhiều lần để giảm nhiễu
    long read_average(int times = 10)
    {
        long sum = 0;
        for (int i = 0; i < times; i++)
        {
            sum += read();
        }
        return sum / times;
    }

    void handleRead()
    {
        long rawValue = read_average(5);
        float newWeight = (rawValue - OFFSET) / SCALE;

        // Kiểm tra dữ liệu hợp lệ (có thể thêm logic kiểm tra khác)
        if (digitalRead(DOUT) != HIGH) // Sensor đang hoạt động
        {
            data.rawValue = rawValue;
            data.weight = newWeight;
            data.isValid = true;
            data.lastReadTime = millis();
        }
        else
        {
            data.isValid = false;
        }
    }

    HX711Data getData() const
    {
        return data;
    }

    bool isWeightInRange() const
    {
        return data.isValid &&
               (data.weight >= weightMin && data.weight <= weightMax);
    }

    bool isInDanger() const
    {
        return data.isValid && !isWeightInRange();
    }

    void setThresholds(float wMin, float wMax)
    {
        weightMin = wMin;
        weightMax = wMax;
    }

    // Đặt hệ số scale
    void set_scale(float scale = 1.0f)
    {
        SCALE = scale;
    }

    // Lấy offset
    long get_offset()
    {
        return OFFSET;
    }

    // Đặt offset
    void set_offset(long offset)
    {
        OFFSET = offset;
    }

    // Thực hiện tare (reset cân về 0)
    void tare(int times = 10)
    {
        long sum = read_average(times);
        set_offset(sum);
        Serial.println("HX711 - Đã thực hiện tare (reset về 0)");
    }

    void log() const
    {
        if (data.isValid)
        {
            Serial.printf("HX711 - Trọng lượng: %.1fg (Raw: %ld)",
                          data.weight, data.rawValue);

            if (isInDanger())
            {
                Serial.print(" [CẢNH BÁO]");
                if (!isWeightInRange())
                {
                    Serial.printf(" - Trọng lượng ngoài phạm vi (%.1f-%.1fg)",
                                  weightMin, weightMax);
                }
            }
            Serial.println();
        }
        else
        {
            Serial.println("HX711 - Lỗi đọc dữ liệu hoặc sensor không sẵn sàng");
        }
    }

    // Các phương thức bổ sung để tương thích với code cũ
    float get_units(int times = 1)
    {
        return (read_average(times) - OFFSET) / SCALE;
    }

    double get_value(int times = 1)
    {
        return read_average(times) - OFFSET;
    }

    bool is_ready()
    {
        return digitalRead(DOUT) == LOW;
    }
};

#endif 