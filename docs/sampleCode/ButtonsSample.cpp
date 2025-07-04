#define DOOR_PIN 4 // GPIO nối với button hoặc reed switch

// Hàm kiểm tra có cần cảnh báo không (giả lập)
bool checkAlert(bool isOpen, unsigned long openDuration) {
    if (isOpen && openDuration >= 10000) {
        // TODO: Thực hiện cảnh báo tại đây
        // Ví dụ: gửi HTTP request, MQTT, hoặc bật còi
        // alertUser(); 
        return true;
    }
    return false;
}

unsigned long openStart = 0;
bool wasOpen = false;

void setup() {
    Serial.begin(115200);
    pinMode(DOOR_PIN, INPUT);
}

void loop() {
    int doorState = digitalRead(DOOR_PIN); // HIGH = mở

    if (doorState == HIGH) {
        if (!wasOpen) {
            wasOpen = true;
            openStart = millis();
            Serial.println("🚪 Cửa đang mở...");
        } else {
            unsigned long openDuration = millis() - openStart;
            if (checkAlert(true, openDuration)) {
                Serial.println("⚠️ CẢNH BÁO: Cửa mở quá lâu (sample)");
            }
        }
    } else {
        if (wasOpen) {
            Serial.println("✅ Cửa đã đóng lại.");
            wasOpen = false;
        }
    }

    delay(500);
}
