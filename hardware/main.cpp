// Biến toàn cục để lưu thông tin cảm biến
float temperature = 0.0;
float humidity = 0.0;
int gasValue = 0;

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 10000; // 10 giây

// Khai báo hàm
bool handleButtonPressRecord(); // Xử lý khi nhấn nút: bắt đầu ghi âm
void recordAudio();             // Ghi âm và lưu dữ liệu âm thanh
void sendAudioToServer();       // Gửi dữ liệu âm thanh lên server

void readTemperatureHumidity();            // Đọc DHT22
void sendTemperatureAndHumidityToServer(); // Gửi nhiệt độ & độ ẩm lên server

void readGasSensor();         // Đọc MQ-135
void sendGasSensorToServer(); // Gửi dữ liệu gas lên server

void updateOLED(float temp, float humi, int gas); // Hiển thị OLED

void setup()
{
    Serial.begin(115200);
    // Khởi tạo cảm biến, OLED, WiFi, v.v.
}

void loop()
{
    readTemperatureHumidity();
    readGasSensor();

    if (handleButtonPressRecord())
    {
        recordAudio();
        sendAudioToServer();
    }

    updateOLED(temperature, humidity, gasValue);

    if (millis() - lastSendTime > SEND_INTERVAL)
    {
        sendTemperatureAndHumidityToServer();
        sendGasSensorToServer();
        lastSendTime = millis();
    }

    delay(1000);
}
