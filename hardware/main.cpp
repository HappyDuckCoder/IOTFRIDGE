#include "button.h"
#include "internet.h"
#include "INMP.h"
#include "constant.h"
#include "gasSensor.h"
#include "handleDelay.h"
#include "DHT.h"
#include "Relay.h"
#include "TFT.h"
#include <SPIFFS.h>

// ============== KHỞI TẠO CÁC OBJECT ==============
Button recordButton(RECORD_BUTTON);
Button fanButton(FAN_BUTTON);
Button tftButton(18); // Nút chuyển trang TFT (có thể thêm vào constant.h)
Internet internet("DRKHOADANG", "1234Dang", "http://192.168.1.11:8888/uploadAudio");
INMP microphone(INMP_WS, INMP_SD, INMP_SCK);

// Cảm biến và điều khiển
GasSensorData gasSensor;
DHTSensor dhtSensor(DHT_PIN, DHT11, 2000);
RelayController fanController(RELAY_PIN, PWM_PIN, 0);
TFTDisplay tftDisplay;

// Timer
HandleDelay gasReadTimer(2000);
HandleDelay dhtReadTimer(2000);
HandleDelay tftUpdateTimer(500); // Cập nhật TFT mỗi 500ms

const char* audioFileName = "/recording.wav";

void onRecordingStateChanged(bool isRecording, int progress)
{
    if (isRecording)
    {
        if (progress == 0)
        {
            Serial.println("Bắt đầu ghi âm...");
            tftDisplay.setRecordingState(true);
        }
        else if (progress % 10 == 0) // Chỉ hiển thị mỗi 10%
        {
            Serial.printf("Tiến trình ghi âm: %d%%\n", progress);
        }
    }
    else
    {
        Serial.println("Ghi âm hoàn thành!");
        tftDisplay.setRecordingState(false);
    }
}

// ============== HÀM SETUP - KHỞI TẠO HỆ THỐNG ==============
void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("=== KHỞI TẠO HỆ THỐNG ESP32 ===");
    Serial.println("Đang khởi tạo các thành phần...");

    // Khởi tạo TFT Display trước tiên
    if (tftDisplay.begin())
    {
        Serial.println("TFT Display khởi tạo thành công.");
    }
    else
    {
        Serial.println("TFT Display khởi tạo lỗi");
    }

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
    Serial.println("Nhấn nút TFT để chuyển trang hiển thị");
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
            tftDisplay.setRecordingState(true);
        }
        else
        {
            // Dừng ghi âm
            Serial.println("Người dùng nhấn nút - Dừng ghi âm");
            microphone.stopRecording();
            tftDisplay.setRecordingState(false);

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
        // TFT sẽ tự động cập nhật trong handleTFT
    }
}

void handleTFTControl()
{
    if (tftButton.isPressed())
    {
        tftDisplay.nextPage();
        Serial.println("Chuyển trang TFT");
    }
}

void handleSensors()
{
    // Xử lý cảm biến DHT11
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

void handleTFT()
{
    // Cập nhật trạng thái kết nối
    tftDisplay.setConnectionState(internet.isConnected());

    // Cập nhật màn hình với dữ liệu từ các cảm biến
    if (tftUpdateTimer.isDue())
    {
        DHTData dhtData = dhtSensor.getData();
        RelayData relayData = fanController.getData();

        tftDisplay.update(dhtData, gasSensor, relayData);
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

// ============== VÒNG LẶP CHÍNH ==============
void loop()
{
    // ========== XỬ LÝ GHI ÂM ==========
    handleVoice();

    // ========== XỬ LÝ ĐIỀU KHIỂN QUẠT ==========
    handleFanControl();

    // ========== XỬ LÝ ĐIỀU KHIỂN TFT ==========
    handleTFTControl();

    // ========== XỬ LÝ CÁC CẢM BIẾN ==========
    handleSensors();

    // ========== XỬ LÝ HIỂN THỊ TFT ==========
    handleTFT();

    // ========== XỬ LÝ CẢNH BÁO ==========
    handleWarnings();

    // Delay nhỏ để giảm tải CPU
    delay(50);
}