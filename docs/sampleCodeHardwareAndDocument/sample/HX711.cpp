#define DT 16
#define SCK 4

void setup()
{
    Serial.begin(115200);
    scale.begin(DT, SCK);
    scale.set_scale(); // cần hiệu chỉnh
    scale.tare();      // đặt 0
}

void loop()
{
    Serial.printf("Khối lượng: %.2f g\n", scale.get_units());
    delay(1000);
}
