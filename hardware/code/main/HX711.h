#ifndef HX711_H
#define HX711_H

#include <Arduino.h>

class HX711 {
public:
    // Số xung thêm để chọn gain/channel theo datasheet
    enum Gain { GAIN_128 = 1, GAIN_64 = 3, GAIN_32 = 2 };

    HX711()
        : _sck(-1), _dout(-1),
          _offset(0), _scale(1.0f),
          _gain(GAIN_128) {}

    HX711(int sck, int dout, Gain gain = GAIN_128)
        : _sck(sck), _dout(dout),
          _offset(0), _scale(1.0f),
          _gain(gain) {}

    // Khởi tạo với chân truyền vào
    bool begin(int sck, int dout, Gain gain = GAIN_128) {
        _sck = sck; _dout = dout; _gain = gain;
        pinMode(_sck, OUTPUT);
        pinMode(_dout, INPUT);
        digitalWrite(_sck, LOW);
        return wait_ready_timeout(400);
    }

    // Khởi tạo dùng chân đã set sẵn
    bool begin() {
        if (_sck < 0 || _dout < 0) return false;
        pinMode(_sck, OUTPUT);
        pinMode(_dout, INPUT);
        digitalWrite(_sck, LOW);
        return wait_ready_timeout(400);
    }

    // API mới
    void set_pins(int sck, int dout) { _sck = sck; _dout = dout; }
    void set_gain(Gain gain) { _gain = gain; if (is_ready()) _pulse_gain(); }
    bool is_ready() const { return digitalRead(_dout) == LOW; }

    bool wait_ready_timeout(uint32_t timeout_ms) const {
        uint32_t start = millis();
        while ((millis() - start) < timeout_ms) {
            if (is_ready()) return true;
            delay(1);
        }
        return false;
    }

    // Đọc 24-bit signed vào out. Trả về true nếu thành công.
    bool read_raw(int32_t &out, uint32_t timeout_ms = 100) {
        if (!wait_ready_timeout(timeout_ms)) return false;

        uint32_t value = 0;
        noInterrupts();
        for (uint8_t i = 0; i < 24; i++) {
            digitalWrite(_sck, HIGH);
            delayMicroseconds(1);
            value = (value << 1) | (uint32_t)digitalRead(_dout);
            digitalWrite(_sck, LOW);
            delayMicroseconds(1);
        }
        _pulse_gain();
        interrupts();

        // sign-extend 24 -> 32
        if (value & 0x800000UL) value |= 0xFF000000UL;
        out = (int32_t)value;
        return true;
    }

    // Trung bình nhiều lần
    bool read_average(int32_t &out, uint8_t times = 10, uint32_t timeout_ms = 100) {
        if (times == 0) times = 1;
        int64_t sum = 0;
        for (uint8_t i = 0; i < times; i++) {
            int32_t v;
            if (!read_raw(v, timeout_ms)) return false;
            sum += v;
        }
        out = (int32_t)(sum / times);
        return true;
    }

    // Giá trị đã trừ offset
    bool get_value(double &out, uint8_t times = 1, uint32_t timeout_ms = 100) {
        int32_t avg;
        if (!read_average(avg, times, timeout_ms)) return false;
        out = (double)avg - (double)_offset;
        return true;
    }

    // API mới: trả về qua tham chiếu, có bool thành công
    bool get_units(float &out, uint8_t times = 1, uint32_t timeout_ms = 100) {
        double val;
        if (!get_value(val, times, timeout_ms)) return false;
        out = (float)(val / (double)_scale);
        return true;
    }

    void tare(uint8_t times = 10, uint32_t timeout_ms = 200) {
        int32_t avg;
        if (read_average(avg, times, timeout_ms)) _offset = avg;
    }

    void set_scale(float s = 1.0f) { _scale = s; }
    float get_scale() const { return _scale; }

    void set_offset(long o) { _offset = o; }
    long get_offset() const { return _offset; }

    void power_down() {
        digitalWrite(_sck, LOW);
        delayMicroseconds(1);
        digitalWrite(_sck, HIGH);
        delayMicroseconds(60);
    }

    void power_up() {
        digitalWrite(_sck, LOW);
        delayMicroseconds(1);
    }

    // ===== Shims tương thích ngược với code cũ =====

    // get_units() kiểu cũ: không tham số, trả về float
    float get_units(int times = 1) {
        float out;
        if (!get_units(out, (uint8_t)times, 100)) return 0.0f;
        return out;
    }

    // set_pin(dout, sck) kiểu cũ
    void set_pin(int dout, int sck) { set_pins(sck, dout); }

    // read() kiểu cũ: trả về long
    long read() {
        int32_t v;
        if (!read_raw(v, 100)) return 0L;
        return (long)v;
    }

private:
    int _sck, _dout;
    long _offset;
    float _scale;
    Gain _gain;

    inline void _pulse_gain() const {
        for (uint8_t i = 0; i < (uint8_t)_gain; i++) {
            digitalWrite(_sck, HIGH);
            delayMicroseconds(1);
            digitalWrite(_sck, LOW);
            delayMicroseconds(1);
        }
    }
};

#endif // HX711_H
