#define MQ135_A_PIN 36 // Chân AOUT nối GPIO36 (analog)
#define MQ135_D_PIN 32 // Chân DOUT nối GPIO32 (digital)

void setup()
{
    Serial.begin(115200);
    pinMode(MQ135_D_PIN, INPUT);
}

void loop()
{
    int analogValue = analogRead(MQ135_A_PIN);   // Đọc giá trị nồng độ
    int digitalState = digitalRead(MQ135_D_PIN); // Đọc trạng thái phát hiện khí

    if (digitalState == LOW)
    {
        Serial.print("⚠️ Có khí thoát ra! ");
        Serial.print("Nồng độ khí (raw): ");
        Serial.print(analogValue);

        // Ước lượng ppm (giả định tuyến tính - cần hiệu chuẩn thật nếu chính xác)
        float ppm = analogValue * (1000.0 / 4095.0); // giả định giới hạn 0–1000 ppm
        Serial.print(" ≈ ");
        Serial.print(ppm);
        Serial.println(" ppm");
    }
    else
    {
        Serial.println("✅ Không có khí phát hiện.");
    }

    delay(1000); // đọc mỗi giây
}
