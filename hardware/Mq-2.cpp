#define MQ2_PIN 34
#define MQ135_PIN 35

const float RL = 10000.0;
const float Vcc = 3.3;

// Tham số đường cong (tra từ datasheet)
const float MQ2_A = -0.38, MQ2_B = 1.48;     // CH4
const float MQ135_A = -0.45, MQ135_B = 2.95; // NH3

// Ngưỡng cảnh báo
const float CH4_THRESHOLD = 3000.0;
const float NH3_THRESHOLD = 50.0;

// Biến hiệu chuẩn
float Ro_MQ2 = 1.0;
float Ro_MQ135 = 1.0;

bool calibrated = false;

float calculateRs(int adc)
{
    float Vout = adc * Vcc / 4095.0;
    return RL * (Vcc - Vout) / Vout;
}

float calculatePPM(float ratio, float a, float b)
{
    return pow(10, (a * log10(ratio) + b));
}

void calibrateRo()
{
    long sum_mq2 = 0, sum_mq135 = 0;
    int samples = 100;
    Serial.println("Hiệu chuẩn Ro trong không khí sạch (10 giây)...");

    for (int i = 0; i < samples; i++)
    {
        sum_mq2 += calculateRs(analogRead(MQ2_PIN));
        sum_mq135 += calculateRs(analogRead(MQ135_PIN));
        delay(100);
    }

    float avg_rs_mq2 = sum_mq2 / (float)samples;
    float avg_rs_mq135 = sum_mq135 / (float)samples;

    Ro_MQ2 = avg_rs_mq2 / 4.4;
    Ro_MQ135 = avg_rs_mq135 / 3.7;

    Serial.print("Ro MQ-2 (CH4): ");
    Serial.println(Ro_MQ2);
    Serial.print("Ro MQ-135 (NH3): ");
    Serial.println(Ro_MQ135);

    calibrated = true;
}

void setup()
{
    Serial.begin(115200);
    delay(2000);
    calibrateRo();
}

void loop()
{
    if (!calibrated)
        return;

    // Đọc MQ-2
    int adc2 = analogRead(MQ2_PIN);
    float rs2 = calculateRs(adc2);
    float ratio_mq2 = rs2 / Ro_MQ2;
    float ppm_ch4 = calculatePPM(ratio_mq2, MQ2_A, MQ2_B);

    // Đọc MQ-135
    int adc135 = analogRead(MQ135_PIN);
    float rs135 = calculateRs(adc135);
    float ratio_mq135 = rs135 / Ro_MQ135;
    float ppm_nh3 = calculatePPM(ratio_mq135, MQ135_A, MQ135_B);

    // In kết quả
    Serial.print("CH4: ");
    Serial.print(ppm_ch4);
    Serial.print(" ppm\t");

    Serial.print("NH3: ");
    Serial.print(ppm_nh3);
    Serial.println(" ppm");

    // Cảnh báo
    if (ppm_ch4 > CH4_THRESHOLD || ppm_nh3 > NH3_THRESHOLD)
    {
        Serial.println("⚠️  CẢNH BÁO: Có thể đồ ăn trong tủ lạnh bị ôi thiu!");
    }
    else
    {
        Serial.println("✅ Mọi thứ ổn.");
    }

    delay(5000);
}
