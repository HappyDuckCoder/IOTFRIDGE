#include <DHT.h>
#include <Wire.h>

#define DHTPIN 23
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Ngưỡng giới hạn
float temp_min = 2.0;
float temp_max = 8.0;
float humid_min = 60.0;
float humid_max = 85.0;

void setup()
{
    Serial.begin(115200);
    dht.begin();
}

void loop()
{
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("Doc cam bien loi!");
    }
    else
    {
        Serial.print("Nhiet do: %.1f C: ");
        Serial.println(temperature);
        Serial.print("Do am:    %.1f %%: ");
        Serial.println(humidity);

        // So sánh với ngưỡng
        if (temperature < temp_min || temperature > temp_max ||
            humidity < humid_min || humidity > humid_max)
        {
            Serial.println("! Canh bao: Vuot nguong");
            // thêm còi / relay nếu cần
        }
        else
        {
            Serial.println("✔ An toan");
        }
    }

    delay(3000);
}