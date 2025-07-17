#include <Arduino.h>

// khai báo chân cảm biến
#define MQ2_PIN 34
#define MQ135_PIN 35

// =======================
// Class xử lý delay không chặn
// =======================
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

// =======================
// Class lưu và xử lý dữ liệu cảm biến khí gas
// =======================
class GasSensorData
{
public:
    int adcMQ2 = 0;
    int adcMQ135 = 0;
    float ppmCH4 = 0.0;
    float ppmNH3 = 0.0;
    float rsMQ2 = 0.0;
    float rsMQ135 = 0.0;
    float ratioMQ2 = 0.0;
    float ratioMQ135 = 0.0;
    float RoMQ2 = 1.0;
    float RoMQ135 = 1.0;
    bool calibrated = false;

    const float RL = 10000.0;
    const float Vcc = 3.3;
    const float MQ2_A = -0.38, MQ2_B = 1.48;
    const float MQ135_A = -0.45, MQ135_B = 2.95;
    const float CH4_THRESHOLD = 3000.0;
    const float NH3_THRESHOLD = 50.0;

    void calibrate()
    {
        long sumMQ2 = 0, sumMQ135 = 0;
        int samples = 100;
        Serial.println("Hiệu chuẩn MQ-2 và MQ-135...");

        for (int i = 0; i < samples; i++)
        {
            sumMQ2 += calculateRs(analogRead(MQ2_PIN));
            sumMQ135 += calculateRs(analogRead(MQ135_PIN));
            delay(100); // vẫn blocking vì chỉ chạy một lần
        }

        RoMQ2 = (sumMQ2 / (float)samples) / 4.4;
        RoMQ135 = (sumMQ135 / (float)samples) / 3.7;

        Serial.printf("✓ Ro MQ-2 = %.2f | Ro MQ-135 = %.2f\n", RoMQ2, RoMQ135);
        calibrated = true;
    }

    void handleRead()
    {
        if (!calibrated)
            return;

        adcMQ2 = analogRead(MQ2_PIN);
        adcMQ135 = analogRead(MQ135_PIN);

        rsMQ2 = calculateRs(adcMQ2);
        rsMQ135 = calculateRs(adcMQ135);

        ratioMQ2 = rsMQ2 / RoMQ2;
        ratioMQ135 = rsMQ135 / RoMQ135;

        ppmCH4 = calculatePPM(ratioMQ2, MQ2_A, MQ2_B);
        ppmNH3 = calculatePPM(ratioMQ135, MQ135_A, MQ135_B);
    }

    bool isInDanger()
    {
        return (ppmCH4 > CH4_THRESHOLD || ppmNH3 > NH3_THRESHOLD);
    }

    void log()
    {
        Serial.printf("CH4: %.1f ppm\tNH3: %.1f ppm\t", ppmCH4, ppmNH3);
        if (isInDanger())
        {
            Serial.println("⚠️  NGUY HIỂM!");
        }
        else
        {
            Serial.println("✅ An toàn.");
        }
    }

private:
    float calculateRs(int adc)
    {
        float Vout = adc * Vcc / 4095.0;
        return RL * (Vcc - Vout) / Vout;
    }

    float calculatePPM(float ratio, float a, float b)
    {
        return pow(10, (a * log10(ratio) + b));
    }
};

// =======================
// Khởi tạo đối tượng
// =======================
GasSensorData gasSensor;
HandleDelay gasReadTimer(2000); // đọc cảm biến mỗi 2 giây

// =======================
// DoorTracking Integration
// =======================
#include "HX711.h"
#define DOOR_BUTTON_PIN 4
#define LOADCELL_DOUT_PIN 32
#define LOADCELL_SCK_PIN 33
#define MAX_DOOR_OPEN_TIME 180000 // 3 phút

HX711 scale;
bool doorOpen = false;
unsigned long doorOpenMillis = 0;
float lastWeight = 0;

void setupDoorTracking()
{
    pinMode(DOOR_BUTTON_PIN, INPUT_PULLUP);
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale();  // cần hiệu chỉnh nếu có giá trị scale riêng
    scale.tare();
    lastWeight = scale.get_units(10);
}

void handleDoorTracking()
{
    bool isClosed = digitalRead(DOOR_BUTTON_PIN) == LOW;

    if (!isClosed)
    {
        if (!doorOpen)
        {
            doorOpen = true;
            doorOpenMillis = millis();
            Serial.println("🚪 Cửa tủ đã mở");
        }

        if (millis() - doorOpenMillis > MAX_DOOR_OPEN_TIME)
        {
            Serial.println("⚠️  Cảnh báo: Cửa mở quá lâu!");
            // Gửi cảnh báo ở đây nếu cần
        }
    }
    else
    {
        if (doorOpen)
        {
            doorOpen = false;
            Serial.println("✅ Cửa đã đóng lại");

            float currentWeight = scale.get_units(10);
            float diff = currentWeight - lastWeight;

            if (abs(diff) > 50)
            {
                Serial.printf("📦 Trọng lượng thay đổi: %.2f g\n", diff);
                if (diff > 0)
                    Serial.println("📝 Yêu cầu nhập thông tin thực phẩm vừa thêm vào.");
                else
                    Serial.println("🗑️ Có thực phẩm được lấy ra.");
            }

            lastWeight = currentWeight;
        }
    }
}

// =======================
// Hàm setup và loop chính
// =======================
void setup()
{
    Serial.begin(115200);
    delay(500);

    gasSensor.calibrate();   // cảm biến khí gas
    setupDoorTracking();     // khởi tạo theo dõi cửa
}

void loop()
{
    if (gasReadTimer.isDue())
    {
        gasSensor.handleRead();
        gasSensor.log();
    }

    handleDoorTracking(); // xử lý theo dõi cửa mỗi vòng lặp
}

