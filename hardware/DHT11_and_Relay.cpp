#include <DHT.h>

#define DHTPIN 15 // chân DATA của cảm biến DHT11
#define DHTTYPE DHT11
#define RELAY_PIN 2 // chân điều khiển relay/quạt (GPIO2)

DHT dht(DHTPIN, DHTTYPE);

// Ngưỡng cảnh báo
float temp_min = 2.0;
float temp_max = 8.0;
float humid_min = 60.0;
float humid_max = 85.0;

void setup()
{
    Serial.begin(115200);
    dht.begin();
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // ban đầu quạt OFF
}

void loop()
{
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("❌ Loi doc cam bien DHT11");
        delay(3000);
        return;
    }

    Serial.printf("🌡 Nhiet do: %.1f C | 💧 Do am: %.1f %%\n", temperature, humidity);

    // Nếu vượt ngưỡng → bật relay (quạt ON)
    if (temperature > temp_max || temperature < temp_min ||
        humidity > humid_max || humidity < humid_min)
    {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("⚠️ Vuot nguong! Quat DANG BAT");
    }
    else
    {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("✅ Trong nguong. Quat DANG TAT");
    }

    delay(3000); // đọc mỗi 3 giây
}