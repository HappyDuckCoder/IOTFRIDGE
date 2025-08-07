#include "constant.h"
// #include "button.h"
// #include "DHTSensor.h"
// #include "GasSensor.h"
#include "HandleDelay.h"
// #include "Relay.h"
#include "TFT.h"
// #include "Spiff.h"
// #include "INMP.h"
// #include "I2SRecorder.h"
// #include "DoorTracking.h"
// #include "WeightTracking.h"
#include "InternetProvisioning.h"

// // =====================Define Object Section====================== //
// Button
// Button button_mic(BUTTON_MIC_PIN);
// Button button_fan(BUTTON_FAN_PIN);
// Button button_door(BUTTON_DOOR_PIN);
// DHT
// DHTSensor dhtSensor(DHT_PIN, DHT11);
// MQ2, MQ135
// GasSensorSystem gasSystem(MQ2_PIN, MQ135_PIN);
// Relay
// RelayController fanRelay(RELAY_PIN);
// ST7789 Display
TFTDisplay tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN, TFT_SCLK_PIN, TFT_MOSI_PIN, TFT_SCREEN_WIDTH, TFT_SCREEN_HEIGHT);
// spiff
// Spiff spiff;
// INMP
// INMP mic(INMP_BCLK_PIN, INMP_WS_PIN, INMP_DATA_PIN);
// Recorder
// I2SRecorder recorder(mic, I2S_READ_LEN, SAMPLE_RATE, SAMPLE_BITS, CHANNEL_NUM);
// DoorTracking
// DoorTracking doorTracker(10000, 3000);
// WeightTracking weightTracker(HX711_DOUT_PIN, HX711_SCK_PIN, 20.0);
// ap mode
InternetProvisioning net;
// TimerReader
// HandleDelay dhtReadTimer(2000);
// HandleDelay gasSystemReadTimer(2000);
// HandleDelay InternetCheckingReadTimer(200);
// HandleDelay SendDataReadTimer(5000);
HandleDelay ReceiveDataReadTimer(5000);
// =====================Define Object Section====================== //

// =====================Support Section====================== //
class HandleFunction
{
private:
  bool record_state; // nếu đang record -> true, không record -> false

public:
  HandleFunction() : record_state(false) {}

  // Xử lý luồng cảm biến đồ ăn bị hư
  // void handleSensors()
  // {
  //   // Xử lý cảm biến khí gas
  //   if (gasSystemReadTimer.isDue())
  //   {
  //     gasSystem.handleRead();
  //     gasSystem.log();
  //     if (gasSystem.isSystemInDanger()) 
  //       net.uploadNotification("Cảnh báo có đồ ăn bị hư", "/uploadNotification");
  //   }
  // }

  // Xử lý luồng nhiệt độ và độ ẩm
  // void handleRelay()
  // {
  //   if (button_fan.isHeldtSecond(5)) // nhấn giữ 5 giây để đổi chế độ
  //   {
  //     bool automationMode = fanRelay.getCurrentAutomationMode();

  //     if (automationMode == true)
  //     {
  //       Serial.println("Đang đổi chế độ tự động");
  //     }
  //     else
  //     {
  //       Serial.println("Đang đổi chế độ thủ công");
  //     }

  //     fanRelay.setAutomationMode(!automationMode);
  //   }

  //   fanRelay.update();

  //   if (fanRelay.getCurrentAutomationMode() == false)
  //   {
  //     if (button_fan.isPressed())
  //     {
  //       fanRelay.nextMode();
  //       fanRelay.log();
  //     }
  //   }
  //   else
  //   {
  //     if (dhtReadTimer.isDue())
  //     {
  //       dhtSensor.handleRead();
  //       float t = dhtSensor.getData().temperature;
  //       fanRelay.updateTemperature(t);
  //     }
  //   }
  // }

  // Xử lý ST7789
  void handleDisplayTFT()
  {
      static FridgeData f(0, 0, 0, 0, 0, 0); // giữ lại giá trị giữa các lần gọi

      if (ReceiveDataReadTimer.isDue()) 
      {
          f = net.readData("/receiveSetting"); // cập nhật nếu đủ 5 giây
      }

      // luôn hiển thị thông số mới nhất hiện có
      tft.showMain(
          f.temp,
          f.humi,
          f.is_rotted_food,
          f.total_food,
          f.last_open,
          f.is_saving_mode
      );
  }

  // xử lý internet
  void handleInternet()
  {
    net.handleClient();
    
    // test
    // if (SendDataReadTimer.isDue())
    // {
    //   net.uploadTestData(100, "/uploadTestData");
    // }
  }

  // test
  // void handleTestSpiff()
  // {
  //   if (button_mic.isPressed())
  //   {
  //     spiff.deleteAllFiles();
  //     spiff.writeFile("/log.txt", "Dữ liệu test SPIFFS\n");
  //     String content = spiff.readFile("/log.txt");
  //     if (spiff.exists("/log.txt"))
  //     {
  //       Serial.print("Nội dung đọc được: ");
  //       Serial.println(content);
  //       spiff.listFiles();
  //     }
  //     spiff.deleteFile("/log.txt");
  //   }
  // }

  // xử lý luồng thêm, xóa thức ăn
  // void handleRecord()
  // {
  //   if (button_mic.isPressed())
  //   {
  //     if (!record_state)
  //     {
  //       record_state = true;
  //       Serial.println("Bắt đầu ghi âm...");
  //       recorder.start("/mic.pcm");
  //     }
  //     else
  //     {
  //       record_state = false;
  //       Serial.println("Đang dừng ghi âm...");
  //       recorder.stop();

  //       delay(300); // đợi file đóng

  //       // send to server
  //       if (net.uploadFile("/mic.pcm", "/uploadAudio"))
  //         Serial.println("Upload OK");
  //       else
  //         Serial.println("Upload lỗi");

  //       // Kiểm tra file tồn tại trước khi xóa
  //       if (spiff.exists("/mic.pcm"))
  //       {
  //         if (!spiff.deleteFile("/mic.pcm"))
  //         {
  //           Serial.println("Lỗi xóa file");
  //         }
  //       }
  //       else
  //       {
  //         Serial.println("File không tồn tại để xóa");
  //       }
  //     }
  //   }
  // }

  // xử lý luồng đóng mở cửa tủ
  // void handleDoorChecking()
  // {
  //   DoorState doorState = button_door.isHeld() == LOW ? DOOR_OPEN : DOOR_CLOSED;
  //   doorTracker.setCurrentState(doorState);

  //   // Serial.print("state: ");
  //   // Serial.println(doorState);

  //   if (doorTracker.isAlertNeeded())
  //   {
  //     Serial.println("[ALERT] Door has been open too long!");
  //     net.uploadNotification("Cảnh báo quên đóng cửa tủ!!!", "/uploadNotification");
  //   }

  //   if (doorTracker.isDoorJustClosed())
  //   {
  //     Serial.println("[INFO] Door just closed quickly.");
  //   }

  //   if (weightTracker.checkWeightChange())
  //   {
  //     Serial.println(2);
  //     Serial.print("[INFO] Weight changed. Current weight: ");
  //     Serial.println(weightTracker.getCurrentWeight());

  //     net.uploadNotification("Cảnh báo quên thêm đồ ăn!!!", "/uploadNotification");
  //   }
  // }
};
HandleFunction handle;
// // =====================Support Section====================== //

// // =====================Loop, Setup Section====================== //
void setup()
{
  Serial.begin(115200);

  // buttons begin
  // if (button_mic.begin())
  //   Serial.println("Button Mic khởi tạo thành công");
  // else
  //   Serial.println("Button Mic khởi tạo thất bại");
  //   if (button_fan.begin())
  //     Serial.println("Button Fan khởi tạo thành công");
  //   else
  //     Serial.println("Button Fan khởi tạo thất bại");
  // if (button_door.begin())
  //   Serial.println("Button Door khởi tạo thành công");
  // else
  //   Serial.println("Button Door khởi tạo thất bại");

  // dht begin
  // if (dhtSensor.begin())
  //   Serial.println("DHT khởi tạo thành công");
  // else
  //   Serial.println("DHT khởi tạo thất bại");

  // gas begin
  // if (gasSystem.begin())
  //   Serial.println("Hệ thống gas khởi tạo thành công");
  // else
  //   Serial.println("Hệ thống gas khởi tạo thất bại");

  // relay begin
  // if (fanRelay.begin())
  //   Serial.println("Relay khởi tạo thành công");
  // else
  //   Serial.println("Relay khởi tạo thất bại");

  // begin ST7789
  if (tft.begin())
    Serial.println("TFT khởi tạo thành công");
  else
    Serial.println("TFT khởi tạo thất bại");

  // spiff Begin
  // if (spiff.begin())
  //   Serial.println("spiff khởi tạo thành công");
  // else
  //   Serial.println("spiff Khởi tạo thất bại");

  // INMP Begin
  // if (mic.begin())
  //   Serial.println("INMP khởi tạo thành công");
  // else
  //   Serial.println("INMP khởi tạo thất bại");

  // HX711 begin
  // if (weightTracker.begin())
  //   Serial.println("cân khởi tạo thành công");
  // else
  //   Serial.println("cân khởi tạo thất bại");

  // ap mode begin
  if (net.begin())
    Serial.println("internet khởi tạo thành công");
  else
    Serial.println("internet khởi tạo thất bại");
}

void loop()
{
  // Xử lý kết nối internet trước
  handle.handleInternet();

  // xử lý hiện thông số ra tft
  handle.handleDisplayTFT();

  // xử lý luồng 1:
  // handle.handleRecord();

  // xử lý luồng 2:
  // handle.handleRelay();

  // xử lý luồng 3:
  // handle.handleDoorChecking();

  // xử lý luồng 4: 
  // handle.handleSensors();

  delay(50);
}
// // =====================Loop, Setup Section====================== //
