#include "button.h"
#include "internet.h"
#include "INMP.h"
#include "constant.h"
#include "gasSensor.h"
#include "handleDelay.h"
#include "DHT.h"
#include "Relay.h"
#include <SPIFFS.h>

// ============== KHỞI TẠO CÁC OBJECT ==============
Button recordButton(RECORD_BUTTON);
Button fanButton(FAN_BUTTON); // Nút điều khiển quạt (cần định nghĩa trong constant.h)
Internet internet("DRKHOADANG", "1234Dang", "http://192.168.1.11:8888/uploadAudio");
INMP microphone(INMP_WS, INMP_SD, INMP_SCK);

// Cảm biến và điều khiển
GasSensorData gasSensor;
DHTSensor dhtSensor(DHT_PIN, DHT11, 2000); // Cần định nghĩa DHT_PIN trong constant.h
RelayController fanController(RELAY_PIN, PWM_PIN, 0); // Cần định nghĩa RELAY_PIN, PWM_PIN trong constant.h

// Timer
HandleDelay gasReadTimer(2000);
HandleDelay dhtReadTimer(2000);
HandleDelay systemStatusTimer(5000); // Hiển thị trạng thái hệ thống mỗi 5 giây

const char* audioFileName = "/recording.wav";

void onRecordingStateChanged(bool isRecording, int progress)
{
    if (isRecording)
    {
        if (progress == 0)
        {
            Serial.println("Bắt đầu ghi âm...");
        }
        else if (progress % 10 == 0) // Chỉ hiển thị mỗi 10%
        {
            Serial.printf("Tiến trình ghi âm: %d%%\n", progress);
        }
    }
    else
    {
        Serial.println("Ghi âm hoàn thành!");
    }
}

// ============== HÀM SETUP - KHỞI TẠO HỆ THỐNG ==============
void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("=== KHỞI TẠO HỆ THỐNG ESP32 ===");
    Serial.println("Đang khởi tạo các thành phần...");

    // Khởi tạo SPIFFS cho lưu trữ file âm thanh
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS khởi tạo lỗi!");
        while (1)
            yield();
    }
    Serial.println("SPIFFS khởi tạo thành công.");

    // Khởi tạo microphone và callback
    if (microphone.begin())
    {
        microphone.setRecordingCallback(onRecordingStateChanged);
        Serial.println("Microphone khởi tạo thành công.");
    }
    else
    {
        Serial.println("Microphone khởi tạo lỗi");
    }

    // Khởi tạo cảm biến DHT11
    if (dhtSensor.begin())
    {
        Serial.println("DHT11 khởi tạo thành công.");
    }
    else
    {
        Serial.println("DHT11 khởi tạo lỗi");
    }

    // Khởi tạo điều khiển quạt
    if (fanController.begin())
    {
        Serial.println("Fan controller khởi tạo thành công.");
    }
    else
    {
        Serial.println("Fan controller khởi tạo lỗi");
    }

    // Khởi tạo kết nối internet
    internet.begin();
    Serial.println("Đang khởi tạo kết nối WiFi...");

    // Khởi tạo và hiệu chuẩn cảm biến khí gas
    Serial.println("Đang hiệu chuẩn cảm biến khí gas.");
    gasSensor.calibrate();
    Serial.println("Cảm biến khí gas đã được hiệu chuẩn.");

    Serial.println("=== HỆ THỐNG SẴN SÀNG ===");
    Serial.println("Nhấn nút ghi âm để bắt đầu/dừng ghi âm");
    Serial.println("Nhấn nút quạt để chuyển chế độ quạt");
    Serial.println("Các cảm biến sẽ đọc dữ liệu tự động");
    Serial.println("================================");
}

// ============== HÀM HỖ TRỢ VÒNG LẶP CHÍNH ==============
void handleVoice()
{
    if (recordButton.isPressed())
    {
        if (!microphone.isRecording())
        {
            // Bắt đầu ghi âm
            Serial.println("Người dùng nhấn nút - Bắt đầu ghi âm");
            microphone.startRecording(audioFileName);
        }
        else
        {
            // Dừng ghi âm
            Serial.println("Người dùng nhấn nút - Dừng ghi âm");
            microphone.stopRecording();

            // Hiển thị danh sách file
            Serial.println("Danh sách file trong SPIFFS:");
            microphone.listFiles();

            // Upload nếu có kết nối internet
            if (internet.isConnected())
            {
                Serial.println("Kết nối internet OK - Bắt đầu upload");
                if (internet.uploadFile(audioFileName))
                {
                    Serial.println("Upload thành công - Xóa file cũ");
                    microphone.clearAllFiles();
                }
                else
                {
                    Serial.println("Upload thất bại - Giữ lại file");
                }
            }
            else
            {
                Serial.println("Chưa kết nối internet, không thể upload");
                Serial.println("File âm thanh đã được lưu local");
            }

            Serial.println("Nhấn nút để ghi âm tiếp theo");
        }
    }
}

void handleFanControl()
{
    if (fanButton.isPressed())
    {
        fanController.nextMode();
    }
}

void handleSensors()
{
    // Xử lý cảm biến DHT22
    if (dhtReadTimer.isDue())
    {
        dhtSensor.handleRead();
    }

    // Xử lý cảm biến khí gas
    if (gasReadTimer.isDue())
    {
        gasSensor.handleRead();
    }
}

void handleWarnings()
{
    // Kiểm tra cảnh báo từ DHT11
    if (dhtSensor.isInDanger())
    {
        Serial.println("CẢNH BÁO DHT11: Nhiệt độ hoặc độ ẩm ngoài phạm vi an toàn!");
    }

    // Kiểm tra cảnh báo từ cảm biến khí gas
    if (gasSensor.isInDanger())
    {
        Serial.println("CẢNH BÁO GAS: Nồng độ khí gas cao, có thực phẩm bị hỏng!");
    }
}

void handleSystemStatus()
{
    if (systemStatusTimer.isDue())
    {
        Serial.println("========== TRẠNG THÁI HỆ THỐNG ==========");

        // Hiển thị thông tin DHT11
        dhtSensor.log();

        // Hiển thị thông tin cảm biến gas
        gasSensor.log();

        // Hiển thị trạng thái quạt
        fanController.log();

        // Hiển thị trạng thái kết nối
        Serial.printf("WiFi: %s\n", internet.isConnected() ? "Đã kết nối" : "Chưa kết nối");

        Serial.println("========================================");
    }
}

// ============== VÒNG LẶP CHÍNH ==============
void loop()
{
    // ========== XỬ LÝ GHI ÂM ==========
    handleVoice();

    // ========== XỬ LÝ ĐIỀU KHIỂN QUẠT ==========
    handleFanControl();

    // ========== XỬ LÝ CÁC CẢM BIẾN ==========
    handleSensors();

    // ========== XỬ LÝ CẢNH BÁO ==========
    handleWarnings();

    // ========== HIỂN THỊ TRẠNG THÁI HỆ THỐNG ==========
    handleSystemStatus();

    // Delay nhỏ để giảm tải CPU
    delay(50);
}