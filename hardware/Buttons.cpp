#define DOOR_PIN 4      // GPIO nối với button/cảm biến cửa
#define RELAY_PIN 2     // GPIO điều khiển relay/quạt/đèn

unsigned long openStart = 0;
bool wasOpen = false;
const unsigned long MAX_OPEN_TIME = 10000; // 10 giây

// Hàm kiểm tra cần cảnh báo không (giả lập)
bool checkAlert(bool isOpen, unsigned long openDuration) {
    if (isOpen && openDuration >= MAX_OPEN_TIME) {
        // TODO: Thực hiện cảnh báo tại đây
        // Ví dụ: gửi cảnh báo lên web, bật buzzer, MQTT...
        // alertUser();
        return true;
    }
    return false;
}

void setup() {
    Serial.begin(115200);
    pinMode(DOOR_PIN, INPUT);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
}

void loop() {
    int doorState = digitalRead(DOOR_PIN); // HIGH = mở

    if (doorState == HIGH) {
        if (!wasOpen) {
            wasOpen = true;
            openStart = millis();
            Serial.println("🚪 Cửa đang mở...");
        } else {
            unsigned long duration = millis() - openStart;
            if (checkAlert(true, duration)) {
                digitalWrite(RELAY_PIN, HIGH); // Bật relay
                Serial.println("⚠️ CẢNH BÁO: Cửa mở quá lâu!");
            }
        }
    } else {
        if (wasOpen) {
            Serial.println("✅ Cửa đã đóng lại.");
            wasOpen = false;
        }
        digitalWrite(RELAY_PIN, LOW); // Tắt relay nếu cửa đã đóng
    }

    delay(200);
}
