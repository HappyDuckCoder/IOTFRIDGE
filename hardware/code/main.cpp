#include "button.h"
#include "internet.h"
#include "INMP.h"
#include "constant.h"
#include "gasSensor.h"
#include "handleDelay.h"
#include <SPIFFS.h>

// ============== KHỞI TẠO CÁC OBJECT ==============
Button recordButton(RECORD_BUTTON);
Internet internet("DRKHOADANG", "1234Dang", "http://192.168.1.11:8888/uploadAudio");
INMP microphone(INMP_WS, INMP_SD, INMP_SCK);

GasSensorData gasSensor;
HandleDelay gasReadTimer(2000);

const char *audioFileName = "/recording.wav";

void onRecordingStateChanged(bool isRecording, int progress)
{
    if (isRecording)
    {
        if (progress == 0)
        {
            Serial.println("🎤 Bắt đầu ghi âm...");
        }
        else if (progress % 10 == 0) // Chỉ hiển thị mỗi 10%
        {
            Serial.printf("📊 Tiến trình ghi âm: %d%%\n", progress);
        }
    }
    else
    {
        Serial.println("✅ Ghi âm hoàn thành!");
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
        Serial.println("microphone khởi tạo lỗi");
    }

    // Khởi tạo kết nối internet
    internet.begin();
    Serial.println("Đang khởi tạo kết nối WiFi...");

    // Khởi tạo và hiệu chuẩn cảm biến khí gas
    Serial.println("Đang hiệu chuẩn cảm biến khí gas.");
    gasSensor.calibrate();
    Serial.println("Cảm biến khí gas đã được hiệu chuẩn.");

    Serial.println("=== HỆ THỐNG SẴN SÀNG ===");
    Serial.println("Nhấn nút để bắt đầu/dừng ghi âm");
    Serial.println("Cảm biến khí gas sẽ đọc dữ liệu mỗi 2 giây");
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

void handleGasSensor()
{
    if (gasReadTimer.isDue())
    {
        gasSensor.handleRead();
        gasSensor.log();

        if (gasSensor.isInDanger())
        {
            Serial.println("CẢNH BÁO: Nồng độ khí gas cao, có thực phẩm bị hỏng!");
            //* NOTE: Gửi cảnh báo qua internet & điện thoại ở đây
        }
    }
}

// ============== VÒNG LẶP CHÍNH ==============
void loop()
{
    // ========== XỬ LÝ GHI ÂM ==========
    handleVoice();

    // ========== XỬ LÝ CẢM BIẾN KHÍ GAS ==========
    handleGasSensor();

    // Delay nhỏ để giảm tải CPU
    delay(50);
}