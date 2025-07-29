#include <DHT.h>

#define DHTPIN 21     // chân DATA của DHT22 nối vào GPIO21 (bạn có thể đổi tùy ý)
#define DHTTYPE DHT22 // dùng loại DHT22

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
    Serial.begin(115200);
    dht.begin();
}

void loop()
{
    delay(2000); // DHT22 cần 2 giây giữa mỗi lần đọc

    float h = dht.readHumidity();
    float t = dht.readTemperature();     // độ C
    float f = dht.readTemperature(true); // độ F (tùy chọn)

    if (isnan(h) || isnan(t))
    {
        Serial.println("Không đọc được dữ liệu từ cảm biến DHT22!");
        return;
    }

    Serial.print("Độ ẩm: ");
    Serial.print(h);
    Serial.print(" %\t");

    Serial.print("Nhiệt độ: ");
    Serial.print(t);
    Serial.println(" °C");
}
