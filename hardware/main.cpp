#include <WiFi.h>
#include <DHT.h>
#include <HX711.h>
#include <Wire.h>
#include <SSD1306Wire.h>

// ===============================
// CẤU HÌNH CHÂN VÀ HẰNG SỐ
// ===============================
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define GAS_PIN A0
#define BUTTON_PIN 2
#define DOOR_SENSOR_PIN 3
#define LOADCELL_DOUT_PIN 5
#define LOADCELL_SCK_PIN 6
#define OLED_SDA 21
#define OLED_SCL 22

// Cấu hình thời gian
const unsigned long SEND_INTERVAL = 10000;       // 10 giây
const unsigned long SENSOR_READ_INTERVAL = 1000; // 1 giây
const unsigned long OLED_UPDATE_INTERVAL = 2000; // 2 giây
const unsigned long DEBOUNCE_DELAY = 50;         // 50ms cho debounce button

// Ngưỡng cảm biến
const float WEIGHT_THRESHOLD = 0.1; // Ngưỡng thay đổi cân nặng (kg)
const int GAS_THRESHOLD = 300;      // Ngưỡng cảnh báo gas

// ===============================
// KHỞI TẠO CÁC THÀNH PHẦN
// ===============================
DHT dht(DHT_PIN, DHT_TYPE);
HX711 scale;
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

// ===============================
// CẤU TRÚC DỮ LIỆU
// ===============================
struct SensorData
{
    float temperature;
    float humidity;
    int gasValue;
    float weightChange;
    bool isDoorOpen;
    bool isRecording;
    unsigned long lastUpdate;

    SensorData() : temperature(0.0), humidity(0.0), gasValue(0),
                   weightChange(0.0), isDoorOpen(false), isRecording(false),
                   lastUpdate(0) {}
};

struct SystemState
{
    unsigned long lastSendTime;
    unsigned long lastSensorRead;
    unsigned long lastOLEDUpdate;
    bool buttonPressed;
    unsigned long buttonPressTime;
    float baselineWeight;
    bool systemReady;

    SystemState() : lastSendTime(0), lastSensorRead(0), lastOLEDUpdate(0),
                    buttonPressed(false), buttonPressTime(0), baselineWeight(0.0),
                    systemReady(false) {}
};

// ===============================
// BIẾN TOÀN CỤC
// ===============================
SensorData sensorData;
SystemState systemState;

// ===============================
// KHỞI TẠO HỆ THỐNG
// ===============================
void setup()
{
    Serial.begin(115200);

    // Khởi tạo chân GPIO
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
    pinMode(GAS_PIN, INPUT);

    // Khởi tạo cảm biến
    initializeSensors();

    // Khởi tạo OLED
    initializeOLED();

    // Khởi tạo WiFi
    initializeWiFi();

    // Thiết lập baseline cho cân
    calibrateScale();

    systemState.systemReady = true;
    Serial.println("Hệ thống đã sẵn sàng!");
}

// ===============================
// VÒNG LẶP CHÍNH
// ===============================
void loop()
{
    if (!systemState.systemReady)
    {
        delay(1000);
        return;
    }

    unsigned long currentTime = millis();

    // Đọc cảm biến theo chu kỳ
    if (currentTime - systemState.lastSensorRead >= SENSOR_READ_INTERVAL)
    {
        readAllSensors();
        systemState.lastSensorRead = currentTime;
    }

    // Xử lý nút bấm
    handleButtonPress();

    // Cập nhật OLED
    if (currentTime - systemState.lastOLEDUpdate >= OLED_UPDATE_INTERVAL)
    {
        updateDisplay();
        systemState.lastOLEDUpdate = currentTime;
    }

    // Gửi dữ liệu lên server
    if (currentTime - systemState.lastSendTime >= SEND_INTERVAL)
    {
        sendDataToServer();
        systemState.lastSendTime = currentTime;
    }

    // Xử lý ghi âm nếu cần
    if (sensorData.isRecording)
    {
        processAudioRecording();
    }

    delay(10); // Tránh watchdog timeout
}

// ===============================
// KHỞI TẠO CÁC THÀNH PHẦN
// ===============================
void initializeSensors()
{
    dht.begin();
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

    // Chờ cảm biến ổn định
    delay(2000);

    if (scale.is_ready())
    {
        scale.set_scale(2280.f); // Cần hiệu chỉnh theo cân thực tế
        scale.tare();
        Serial.println("Cân đã sẵn sàng");
    }
    else
    {
        Serial.println("Lỗi: Không thể khởi tạo cân");
    }
}

void initializeOLED()
{
}

void initializeWiFi()
{
}

void calibrateScale()
{
}

// ===============================
// ĐỌC CẢM BIẾN
// ===============================
void readAllSensors()
{
    readTemperatureHumidity();
    readGasSensor();
    readDoorStatus();

    if (sensorData.isDoorOpen)
    {
        readWeightChange();
    }
}

void readTemperatureHumidity()
{
}

void readGasSensor()
{
}

void readDoorStatus()
{
}

void readWeightChange()
{
}

// ===============================
// XỬ LÝ NÚT BẤM
// ===============================
void handleButtonPress()
{
}

void startAudioRecording()
{
}

void stopAudioRecording()
{
}

void processAudioRecording()
{
    // Xử lý ghi âm realtime
    // Thêm code ghi âm ở đây
}

// ===============================
// HIỂN THỊ OLED
// ===============================
void updateDisplay()
{
}

// ===============================
// GỬI DỮ LIỆU LÊN SERVER
// ===============================
void sendDataToServer()
{
}

void sendTemperatureAndHumidityToServer()
{
}

void sendGasSensorToServer()

// In ra chuỗi JSON
{
}

void sendWeightChangeToServer()
{
}

void sendAudioToServer()
{
}