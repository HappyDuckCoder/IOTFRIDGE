#define MQ2_ANALOG_PIN 35 // Chỉ cần 1 chân analog

void setup()
{
    Serial.begin(115200);
    Serial.println("MQ-2 - Đọc Analog & Chuyển sang PPM");
    Serial.println("===================================");
}

void loop()
{
    // Đọc giá trị analog từ MQ-2
    int analogValue = analogRead(MQ2_ANALOG_PIN);

    // Chuyển sang điện áp
    float voltage = analogValue * (3.3 / 4095.0);

    // Chuyển sang PPM (công thức ước tính)
    float ppm = convertToPPM(analogValue);

    // Đánh giá mức độ khí
    String gasLevel = getGasLevel(analogValue);
    String warning = getWarning(ppm);

    // Hiển thị kết quả
    Serial.println("📊 KẾT QUẢ ĐO:");
    Serial.print("   Analog: ");
    Serial.print(analogValue);
    Serial.print("/4095");

    Serial.print(" | Voltage: ");
    Serial.print(voltage, 2);
    Serial.println("V");

    Serial.print("   PPM: ");
    Serial.print(ppm, 0);
    Serial.print(" ppm");

    Serial.print(" | Mức độ: ");
    Serial.println(gasLevel);

    Serial.print("   Trạng thái: ");
    Serial.println(warning);

    Serial.println("-----------------------------------");
    delay(2000); // Đọc mỗi 2 giây
}

// Hàm chuyển đổi giá trị analog sang PPM
float convertToPPM(int analogValue)
{
    // Công thức ước tính: giá trị càng thấp = PPM càng cao
    // Mapping từ 4095-0 sang 0-5000 ppm
    float ppm = map(analogValue, 4095, 0, 0, 5000);

    // Giới hạn giá trị âm
    if (ppm < 0)
        ppm = 0;

    return ppm;
}

// Hàm đánh giá mức độ khí
String getGasLevel(int analogValue)
{
    if (analogValue < 1200)
    {
        return "🟢 RẤT ÍT KHÍ";
    }
    else if (analogValue < 2000)
    {
        return "🟡 ÍT KHÍ";
    }
    else if (analogValue < 2800)
    {
        return "🟠 TRUNG BÌNH";
    }
    else if (analogValue < 3500)
    {
        return "🔴 NHIỀU KHÍ";
    }
    else
    {
        return "🚨 RẤT NHIỀU KHÍ";
    }
}

// Hàm cảnh báo
String getWarning(float ppm)
{
    if (ppm < 300)
    {
        return "✅ AN TOÀN";
    }
    else if (ppm < 2000)
    {
        return "⚠️ CHÚ Ý";
    }
    else if (ppm < 3000)
    {
        return "🔥 CẢNH BÁO";
    }
    else
    {
        return "🚨 NGUY HIỂM - THOÁT NGAY!";
    }
}