#include "constant.h"
#include "button.h"
#include "DHTSensor.h"
#include "GasSensor.h"
#include "HandleDelay.h"

// =====================Define Object Section====================== //
// Button 
Button button_mic(BUTTON_MIC_PIN);
Button button_fan(BUTTON_FAN_PIN);
Button button_door(BUTTON_DOOR_PIN);
// DHT
DHTSensor dhtSensor(DHT_PIN, DHT11);
// MQ2, MQ135
GasSensorSystem gasSystem(MQ2_PIN, MQ135_PIN);
// TimerReader
HandleDelay dhtReadTimer(2000);
HandleDelay gasSystemReadTimer(2000);
// =====================Define Object Section====================== //

// =====================Debug Section====================== //
// Debug
class Debug
{
public:
    Debug() {}

    void debugButton()
    {
      if (button_mic.isPressed()) Serial.println("button mic press");
      if (button_fan.isPressed()) Serial.println("button fan press");
      if (button_door.isPressed()) Serial.println("button door press");
    }

    void debugDHT()
    {
      dhtSensor.handleRead();
      dhtSensor.log();
    }

    void debugGasSystem() 
    {
      gasSystem.handleRead();
      gasSystem.log();
    }
};
Debug debug;
// =====================Debug Section====================== //

// =====================Support Section====================== //
class HandleFunction 
{
public:
  HandleFunction() {}

  void handleSensors()
  {
    // Xử lý cảm biến DHT11
    // if (dhtReadTimer.isDue())
    // {
    //   dhtSensor.handleRead();
    //   dhtSensor.log();
    // }

    // Xử lý cảm biến khí gas
    if (gasSystemReadTimer.isDue())
    {
      gasSystem.handleRead();
      gasSystem.log();
    }
  }
};
HandleFunction handle;
// =====================Support Section====================== //

// =====================Loop, Setup Section====================== //
void setup() 
{
  // serial begin
  Serial.begin(115200);

  // buttons begin
  if (button_mic.begin()) Serial.println("Button Mic khởi tạo thành công");
  else Serial.println("Button Mic khởi tạo thất bại");
  if (button_fan.begin()) Serial.println("Button Fan khởi tạo thành công");  
  else Serial.println("Button Fan khởi tạo thất bại");
  if (button_door.begin()) Serial.println("Button Door khởi tạo thành công");  
  else Serial.println("Button Door khởi tạo thất bại");

  // dht begin
  if (dhtSensor.begin()) Serial.println("DHT khởi tạo thành công");  
  else Serial.println("DHT khởi tạo thất bại");

  // gas begin
  if (gasSystem.begin()) Serial.println("Hệ thống gas khởi tạo thành công");  
  else Serial.println("Hệ thống gas  khởi tạo thất bại");
}

void loop() 
{
  handle.handleSensors();

  delay(50);
}
// =====================Loop, Setup Section====================== //
