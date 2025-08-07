#ifndef HX711_H
#define HX711_H

#include <Arduino.h>

class HX711
{
private:
    int PD_SCK;  // Pin Clock
    int DOUT;    // Pin Data
    long OFFSET; // Giá trị offset (tare)
    float SCALE; // Hệ số scale

public:
    HX711() : PD_SCK(0), DOUT(0), OFFSET(0), SCALE(1.0) {}
    HX711(int sck, int dout) : PD_SCK(sck), DOUT(dout), OFFSET(0), SCALE(1.0) {}

    // Khởi tạo chân giao tiếp
    bool begin()
    {
        pinMode(PD_SCK, OUTPUT);
        pinMode(DOUT, INPUT);
        return true;
    }

    void set_pin(int sck, int dout)
    {
        PD_SCK = sck;
        DOUT = dout;
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

    // Lấy giá trị đã trừ offset
    double get_value(int times = 1)
    {
        return read_average(times) - OFFSET;
    }

    // Lấy giá trị đã tính trọng lượng (đơn vị gram hoặc kg tùy scale)
    float get_units(int times = 1)
    {
        return get_value(times) / SCALE;
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
    }
};

#endif // HX711_H