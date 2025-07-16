#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <DHT.h>

// ==== Pin cấu hình ====
#define DHT_PIN 15
#define DHT_TYPE DHT11
#define RELAY_PIN 16
#define MQ2_PIN 36   // VP - Analog
#define MQ135_PIN 39 // VN - Analog

#define TFT_CS 5
#define TFT_RST 4
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_SCLK 18

// ==== Thiết bị ====
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// ==== Timing ====
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 2000; // 2 giây

// ==== Struct dữ liệu cảm biến DHT11 ====
struct SensorData
{
    float temperature;
    float humidity;
    bool isValid;
};

// ==== Struct dữ liệu khí gas ====
struct GasSensorData
{
    int mq2Raw;
    int mq135Raw;
    bool dangerMQ2;
    bool dangerMQ135;
};

// ==== Đọc DHT11 ====
SensorData handleDHT()
{
    SensorData data;
    data.temperature = dht.readTemperature();
    data.humidity = dht.readHumidity();
    data.isValid = !(isnan(data.temperature) || isnan(data.humidity));
    return data;
}

// ==== Hiển thị lên ST7789 ====
void handleTFT(const SensorData &data, const GasSensorData &gas)
{
    tft.fillRect(0, 60, 240, 160, ST77XX_BLACK);

    tft.setCursor(20, 70);
    tft.setTextColor(ST77XX_CYAN);
    tft.printf("Nhiet do: %.1f C", data.temperature);

    tft.setCursor(20, 100);
    tft.printf("Do am: %.1f %%", data.humidity);

    tft.setCursor(20, 130);
    tft.setTextColor(gas.dangerMQ2 ? ST77XX_RED : ST77XX_GREEN);
    tft.printf("MQ-2: %d", gas.mq2Raw);

    tft.setCursor(20, 160);
    tft.setTextColor(gas.dangerMQ135 ? ST77XX_RED : ST77XX_GREEN);
    tft.printf("MQ-135: %d", gas.mq135Raw);
}

// ==== Điều khiển relay ====
void handleRelay(float temperature, bool gasDanger)
{
    if (temperature > 30.0 || gasDanger)
    {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("🚨 Cảnh báo! Bật relay.");
    }
    else
    {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("✅ Bình thường. Tắt relay.");
    }
}

// ==== Đọc MQ-2 & MQ-135 ====
GasSensorData handleMQ(int mq2Threshold = 3000, int mq135Threshold = 3000)
{
    GasSensorData data;

    data.mq2Raw = analogRead(MQ2_PIN);
    data.mq135Raw = analogRead(MQ135_PIN);

    data.dangerMQ2 = data.mq2Raw > mq2Threshold;
    data.dangerMQ135 = data.mq135Raw > mq135Threshold;

    Serial.print("[MQ-2] Raw: ");
    Serial.print(data.mq2Raw);
    if (data.dangerMQ2)
        Serial.print(" ⚠️");

    Serial.print(" | [MQ-135] Raw: ");
    Serial.print(data.mq135Raw);
    if (data.dangerMQ135)
        Serial.print(" ⚠️");
    Serial.println();

    return data;
}

// ==== Kiểm tra delay không chặn ====
bool handleDelay(unsigned long interval)
{
    unsigned long now = millis();
    if (now - lastUpdate >= interval)
    {
        lastUpdate = now;
        return true;
    }
    return false;
}

// ==== Setup ban đầu ====
void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("🔧 Đang khởi động hệ thống...");

    // === Relay ===
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("✅ Relay sẵn sàng");

    // === DHT11 ===
    dht.begin();
    delay(2000); // đợi ổn định
    float testTemp = dht.readTemperature();
    if (isnan(testTemp))
    {
        Serial.println("❌ Không thể đọc từ DHT11! Kiểm tra kết nối.");
    }
    else
    {
        Serial.printf("✅ DHT11 hoạt động. Nhiệt độ đầu tiên: %.1f°C\n", testTemp);
    }

    // === TFT ST7789 ===
    tft.init(240, 240);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 20);
    tft.println("ESP32 IoT Gas Monitor");
    Serial.println("✅ Màn hình TFT khởi tạo xong");

    // === MQ Test Read ===
    int testMQ2 = analogRead(MQ2_PIN);
    int testMQ135 = analogRead(MQ135_PIN);
    if (testMQ2 < 50 && testMQ135 < 50)
    {
        Serial.println("❌ MQ-2 & MQ-135 có thể không kết nối đúng.");
    }
    else
    {
        Serial.printf("✅ MQ-2: %d | MQ-135: %d\n", testMQ2, testMQ135);
    }

    Serial.println("✅ Hệ thống sẵn sàng.");
}

// ==== Loop chính ====
void loop()
{
    if (handleDelay(updateInterval))
    {
        SensorData dhtData = handleDHT();
        GasSensorData gasData = handleMQ();

        if (dhtData.isValid)
        {
            Serial.printf("🌡 %.1f°C | 💧 %.1f%%\n", dhtData.temperature, dhtData.humidity);
            handleRelay(dhtData.temperature, gasData.dangerMQ2 || gasData.dangerMQ135);
            handleTFT(dhtData, gasData);
        }
        else
        {
            Serial.println("❌ Không thể đọc dữ liệu từ DHT11 trong loop.");
        }
    }
}
