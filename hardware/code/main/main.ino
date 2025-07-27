#include "constant.h"
#include "button.h"
#include "DHTSensor.h"
#include "GasSensor.h"
#include "HandleDelay.h"
#include "Relay.h"
#include "TFT.h"

// =====================Define Object Section====================== //
// Button
Button button_mic(BUTTON_MIC_PIN);
Button button_fan(BUTTON_FAN_PIN);
Button button_door(BUTTON_DOOR_PIN);
// DHT
DHTSensor dhtSensor(DHT_PIN, DHT11);
// MQ2, MQ135
GasSensorSystem gasSystem(MQ2_PIN, MQ135_PIN);
// Relay
RelayController fanRelay(RELAY_PIN);
// ST7789 Display
TFTDisplay tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN, TFT_SCLK_PIN, TFT_MOSI_PIN, TFT_SCREEN_WIDTH, TFT_SCREEN_HEIGHT);
// TimerReader
HandleDelay dhtReadTimer(2000);
HandleDelay gasSystemReadTimer(2000);
// =====================Define Object Section====================== //

// =====================Support Section====================== //
class HandleFunction
{
public:
  HandleFunction() {}

  // Xử lý 2 gas sensor và 1 dht sensor
  void handleSensors()
  {
    // Xử lý cảm biến DHT11
    if (dhtReadTimer.isDue())
    {
      dhtSensor.handleRead();
      dhtSensor.log();
    }

    // Xử lý cảm biến khí gas
    if (gasSystemReadTimer.isDue())
    {
      gasSystem.handleRead();
      gasSystem.log();
    }
  }

  // Xử lý relay
  void handleRelay()
  {
    fanRelay.update();
    if (button_fan.isPressed())
    {
      fanRelay.nextMode();
      fanRelay.log();
    }
  }

  // Xử lý ST7789 
  void handleDisplayTFT() 
  {
    // để tạm thời
    tft.showMain(0, 0, 0, 0, 0);
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
  if (button_mic.begin())
    Serial.println("Button Mic khởi tạo thành công");
  else
    Serial.println("Button Mic khởi tạo thất bại");
  if (button_fan.begin())
    Serial.println("Button Fan khởi tạo thành công");
  else
    Serial.println("Button Fan khởi tạo thất bại");
  if (button_door.begin())
    Serial.println("Button Door khởi tạo thành công");
  else
    Serial.println("Button Door khởi tạo thất bại");

  // dht begin
  if (dhtSensor.begin())
    Serial.println("DHT khởi tạo thành công");
  else
    Serial.println("DHT khởi tạo thất bại");

  // gas begin
  if (gasSystem.begin())
    Serial.println("Hệ thống gas khởi tạo thành công");
  else
    Serial.println("Hệ thống gas khởi tạo thất bại");
  // gasSystem.calibrate(); // hiệu chuẩn hóa gas sensor, tạm thời ẩn đi

  // relay begin
  if (fanRelay.begin())
    Serial.println("Relay khởi tạo thành công");
  else
    Serial.println("Relay khởi tạo thất bại");

  // begin ST7789
  if (tft.begin()) 
    Serial.println("TFT khởi tạo thành công");
  else 
    Serial.println("TFT Khởi tạo thất bại");
}

void loop()
{
  handle.handleDisplayTFT();

  delay(50);
}
// =====================Loop, Setup Section====================== //
