#include "GasSensor.h"
#include "HandleDelay.h"

// =======================
// Khởi tạo đối tượng
// =======================
GasSensor gasSensor;
HandleDelay gasReadTimer(2000); // đọc cảm biến mỗi 2 giây

// =======================
// Hàm setup và loop chính
// =======================
void setup()
{
    Serial.begin(115200);
    delay(500);

    gasSensor.setup();
}

void loop()
{
    if (gasReadTimer.isDue())
    {
        gasSensor.handleRead();
        gasSensor.log();
    }

    // nơi bạn có thể thêm các chức năng khác: WiFi, MQTT, Màn hình, v.v.
}
