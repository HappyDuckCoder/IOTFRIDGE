#include "DoorTracking.h"
#include "WeightTracking.h"

// Pin setup
#define BUTTON_PIN 25
#define HX711_DOUT 26
#define HX711_SCK 27

DoorTracking door(BUTTON_PIN);
WeightTracking weight(HX711_DOUT, HX711_SCK);

void setup()
{
    Serial.begin(115200);
    door.begin();
    weight.begin();
}

void loop()
{
    if (door.isAlertNeeded())
    {
        Serial.println("ALERT: Door open too long!");
        // TODO: Gửi thông báo lên server
    }

    if (door.isDoorJustClosed())
    {
        Serial.println("Door just closed. Checking weight...");
        if (weight.checkWeightChange())
        {
            Serial.println("Weight changed significantly!");
            // TODO: Gửi thông báo yêu cầu nhập/chỉnh sửa thực phẩm trên website
        }
    }

    delay(200);
}

// GPIO 25 → Button(door sensor).

//     GPIO 26 → HX711 Data(DOUT)
//         .

//     GPIO 27 → HX711 Clock(SCK)
//         .
