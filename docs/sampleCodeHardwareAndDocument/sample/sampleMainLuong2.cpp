#include <Arduino.h>
#include "HX711.h"
#include "DoorTracking.h"
#include "WeightTracking.h"

#define HX711_DOUT 4
#define HX711_SCK 5

#define DOOR_SENSOR_PIN 2

DoorTracking doorTracker(10000, 3000);                     // alertThreshold = 10s, justClosedThreshold = 3s
WeightTracking weightTracker(HX711_DOUT, HX711_SCK, 20.0); // Threshold = 20g

void setup()
{
    Serial.begin(9600);

    pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); // Giả lập sensor cửa (nút bấm hoặc reed switch)

    Serial.println("=== Door & Weight Tracking Demo ===");

    // Khởi tạo load cell
    if (weightTracker.begin())
    {
        Serial.println("WeightTracking initialized successfully.");
    }
    else
    {
        Serial.println("WeightTracking init failed!");
    }
}

void loop()
{
    DoorState doorState = digitalRead(DOOR_SENSOR_PIN) == HIGH ? DOOR_OPEN : DOOR_CLOSED;
    doorTracker.setCurrentState(doorState);

    doorTracker.log();

    if (doorTracker.isAlertNeeded())
    {
        Serial.println("[ALERT] Door has been open too long!");
    }

    if (doorTracker.isDoorJustClosed())
    {
        Serial.println("[INFO] Door just closed quickly.");
    }

    if (weightTracker.checkWeightChange())
    {
        Serial.print("[INFO] Weight changed. Current weight: ");
        Serial.println(weightTracker.getCurrentWeight());
    }

    delay(500); // Delay nhẹ để dễ quan sát log
}
