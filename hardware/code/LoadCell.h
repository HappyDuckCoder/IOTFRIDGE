#ifndef LOADCELL_H
#define LOADCELL_H

class LoadCell
{
private:
public:
    bool begin()
    {
        pinMode(DOOR_BUTTON_PIN, INPUT_PULLUP);
        scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
        scale.set_scale(); // cần hiệu chỉnh nếu có giá trị scale riêng
        scale.tare();
        lastWeight = scale.get_units(10);
        return true;
    }

    void handleDoorTracking()
    {
        bool isClosed = digitalRead(DOOR_BUTTON_PIN) == LOW;

        if (!isClosed)
        {
            if (!doorOpen)
            {
                doorOpen = true;
                doorOpenMillis = millis();
                Serial.println("🚪 Cửa tủ đã mở");
            }

            if (millis() - doorOpenMillis > MAX_DOOR_OPEN_TIME)
            {
                Serial.println("⚠️  Cảnh báo: Cửa mở quá lâu!");
                // Gửi cảnh báo ở đây nếu cần
            }
        }
        else
        {
            if (doorOpen)
            {
                doorOpen = false;
                Serial.println("✅ Cửa đã đóng lại");

                float currentWeight = scale.get_units(10);
                float diff = currentWeight - lastWeight;

                if (abs(diff) > 50)
                {
                    Serial.printf("📦 Trọng lượng thay đổi: %.2f g\n", diff);
                    if (diff > 0)
                        Serial.println("📝 Yêu cầu nhập thông tin thực phẩm vừa thêm vào.");
                    else
                        Serial.println("🗑️ Có thực phẩm được lấy ra.");
                }

                lastWeight = currentWeight;
            }
        }
    }
};

#endif