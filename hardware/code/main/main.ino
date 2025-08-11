#include "constant.h"
#include "button.h"
// #include "DHTSensor.h"
// #include "GasSensor.h"
#include "HandleDelay.h"
// #include "Relay.h"
// #include "TFT.h"
// #include "Spiff.h"
// #include "INMP.h"
// #include "I2SRecorder.h"
#include "InternetProvisioning.h"
// #include "WeightTracking.h"
// #include "DoorTracking.h"

// =====================Define Object Section====================== //
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
// TFTDisplay tft(TFT_CS_PIN, TFT_DC_PIN, TFT_RST_PIN, TFT_SCLK_PIN, TFT_MOSI_PIN, TFT_SCREEN_WIDTH, TFT_SCREEN_HEIGHT);
// spiff
// Spiff spiff;
// INMP
// INMP mic(INMP_BCLK_PIN, INMP_WS_PIN, INMP_DATA_PIN);
// Recorder
// I2SRecorder recorder(mic, I2S_READ_LEN, SAMPLE_RATE, SAMPLE_BITS, CHANNEL_NUM);
// ap mode
InternetProvisioning net;
// weight + door
// DoorTracking doorTracker(10000, 3000);                     // alertThreshold = 10s, justClosedThreshold = 3s
// WeightTracking weightTracker(HX711_DOUT_PIN, HX711_SCK_PIN, 20.0); // Threshold = 20g

// TimerReader - Điều chỉnh timer cho recording mode
// HandleDelay dhtReadTimer(2000);
// HandleDelay gasSystemReadTimer(2000);
// HandleDelay ReceiveDataReadTimer(5000);
HandleDelay SendDataReadTimer(5000);

// Recording timer - thêm timer riêng cho recording
// HandleDelay recordingStatusTimer(100); // check recording status mỗi 100ms
// =====================Define Object Section====================== //

// =====================Support Section====================== //
class HandleFunction
{
private:
  bool record_state; // nếu đang record -> true, không record -> false
  bool is_recording_mode; // chế độ recording để pause các sensor khác

public:
  HandleFunction() : record_state(false), is_recording_mode(false) {}

  // bool get_is_recording_mode() 
  // {
    // return is_recording_mode;
  // }

  // Kiểm tra trạng thái recording
  // void updateRecordingMode() {
  //   if (recordingStatusTimer.isDue()) {
  //     is_recording_mode = record_state;
  //   }
  // }

  // Xử lý luồng cảm biến đồ ăn bị hư - PAUSE khi đang recording
  // void handleSensors()
  // {
  //   // SKIP nếu đang recording để tránh xung đột ADC
  //   if (is_recording_mode) {
  //     return;
  //   }

  //   // Xử lý cảm biến khí gas
  //   if (gasSystemReadTimer.isDue())
  //   {
  //     gasSystem.handleRead();
  //     gasSystem.log();
  //     if (gasSystem.isSystemInDanger()) 
  //       net.uploadNotification("Cảnh báo có đồ ăn bị hư", "/uploadNotification");
  //   }
  // }

  // Xử lý luồng nhiệt độ và độ ẩm - PAUSE khi đang recording
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
  //     // SKIP DHT reading nếu đang recording để tránh xung đột
  //     if (is_recording_mode) {
  //       return;
  //     }

  //     if (dhtReadTimer.isDue())
  //     {
  //       dhtSensor.handleRead();
  //       float t = dhtSensor.getData().temperature;
  //       Serial.print("Nhiệt độ hiện tại: ");
  //       Serial.println(t);
  //       fanRelay.updateTemperature(t);
  //     }
  //   }
  // }

  // Xử lý ST7789 - LUÔN CHẠY (không cần ADC)
  // void handleDisplayTFT()
  // {
  //     static FridgeData f(0, 0, 0, 0, 0, 0); // giữ lại giá trị giữa các lần gọi

  //     // Hiển thị trạng thái recording
  //     if (is_recording_mode) {
  //       return;
  //     }

  //     if (ReceiveDataReadTimer.isDue()) 
  //     {
  //         f = net.readData("/receiveSetting"); // cập nhật nếu đủ 5 giây
  //     }

  //     // luôn hiển thị thông số mới nhất hiện có
  //     tft.showMain(
  //         f.temp,
  //         f.humi,
  //         f.is_rotted_food,
  //         f.total_food,
  //         f.last_open,
  //         f.is_saving_mode
  //     );
  // }

  // xử lý internet - LUÔN CHẠY
  void handleInternet()
  {
    net.handleClient();
  
    if (SendDataReadTimer.isDue())
    {
      FridgeData f(0, 0, 0, 0, 0, 0);
      // f.temp = dhtSensor.getData().temperature;
      // f.humi = dhtSensor.getData().humidity;
      // f.is_rotted_food = gasSystem.isInDanger();
      // f.total_food = 0;
      // f.is_saving_mode = fanRelay.getCurrentAutomationMode();
      // f.last_open = doorTracker.getLastTimeOpen();

      net.uploadSensorData(f.temp, f.humi, f.is_rotted_food, f.total_food, f.last_open, f.is_saving_mode);
    }
  }

  // void handleDoorTracking()
  // {

  //     DoorState doorState = button_door.isHeld() ? DOOR_CLOSED : DOOR_OPEN;
  //     doorTracker.setCurrentState(doorState);

  //     if (doorState == DOOR_OPEN)
  //     {
  //         if (doorTracker.isAlertNeeded())
  //         {
  //             Serial.println("[ALERT] Door has been open too long!");
  //         }
  //         return; 
  //     }

  //     if (doorTracker.isDoorJustClosed())
  //     {
  //         Serial.println("[INFO] Door just closed quickly.");
  //     }

  //     if (weightTracker.checkWeightChange())
  //     {
  //         Serial.print("[INFO] Weight changed. Current weight: ");
  //         Serial.println(weightTracker.getCurrentWeight());
  //     }
  // }


  // xử lý luồng thêm, xóa thức ăn - CHÍNH
  // void handleRecord()
  // {
  //   if (button_mic.isPressed())
  //   {
  //     if (!record_state)
  //     {
  //       // BẮT ĐẦU RECORDING
  //       record_state = true;
  //       is_recording_mode = true; // Set mode ngay lập tức
        
  //       Serial.println("Bắt đầu ghi âm...");
  //       Serial.println(">>> PAUSE CÁC LUỒNG KHÁC <<<");
        
  //       // Delay ngắn để các sensor dừng hoàn toàn
  //       delay(200);
        
  //       recorder.start("/mic.pcm");
  //     }
  //     else
  //     {
  //       // DỪNG RECORDING
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

  //       // Delay trước khi resume sensors
  //       delay(500);
  //       is_recording_mode = false; // Resume các sensor khác
  //       Serial.println(">>> RESUME CÁC LUỒNG KHÁC <<<");
  //     }
  //   }
  // }
};
HandleFunction handle;
// =====================Support Section====================== //

// =====================Loop, Setup Section====================== //
void setup()
{
  Serial.begin(115200);

  // buttons begin
  // if (button_mic.begin())
  //   Serial.println("Button Mic khởi tạo thành công");
  // else
  //   Serial.println("Button Mic khởi tạo thất bại");
  // if (button_fan.begin())
  //   Serial.println("Button Fan khởi tạo thành công");
  // else
  //   Serial.println("Button Fan khởi tạo thất bại");
  // if (button_door.begin())
  //   Serial.println("Button Door khởi tạo thành công");
  // else
  //   Serial.println("Button Door khởi tạo thất bại");

  // gas begin
  // if (gasSystem.begin())
  //   Serial.println("Hệ thống gas khởi tạo thành công");
  // else
  //   Serial.println("Hệ thống gas khởi tạo thất bại");

  // dht begin
  // if (dhtSensor.begin())
  //   Serial.println("DHT khởi tạo thành công");
  // else
  //   Serial.println("DHT khởi tạo thất bại");

  // relay begin
  // if (fanRelay.begin())
  //   Serial.println("Relay khởi tạo thành công");
  // else
  //   Serial.println("Relay khởi tạo thất bại");

  // begin ST7789
  // if (tft.begin())
  //   Serial.println("TFT khởi tạo thành công");
  // else
  //   Serial.println("TFT khởi tạo thất bại");

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

  // weight begin 
  // if (weightTracker.begin())
  //   Serial.println("cân khởi tạo thành công");
  // else
  //   Serial.println("cân khởi tạo thất bại");

  // ap mode begin
  if (net.begin())
    Serial.println("internet khởi tạo thành công");
  else
    Serial.println("internet khởi tạo thất bại");
  net.setServerBaseURL("http://192.168.1.9:8888");

  Serial.println("=== HỆ THỐNG SẴN SÀNG ===");
}

void loop()
{
  // Update recording mode status
  // handle.updateRecordingMode();

  // Xử lý kết nối internet trước - LUÔN CHẠY
  handle.handleInternet();

  // xử lý hiện thông số ra tft - LUÔN CHẠY
  // handle.handleDisplayTFT();

  // xử lý luồng 1: ghi âm, thêm hoặc xóa food - ƯU TIÊN CAO
  // handle.handleRecord();

  // xử lý luồng 2: điều chỉnh nhiệt độ phù hợp - PAUSE khi recording
  // handle.handleRelay();

  // xử lý luồng 3: kiểm tra xem đã thêm thức ăn hay chưa - PAUSE khi recording
  // handle.handleDoorTracking();

  // xử lý luồng 4: cảnh báo có đồ ăn bị hư - PAUSE khi recording
  // handle.handleSensors();

  // Giảm delay khi đang recording để responsive hơn
  // if (handle.get_is_recording_mode()) {
    // delay(20); // delay ngắn 
  // } else {
    delay(50); // delay bình thường để cpu đỡ quá tải
  // }
}
// =====================Loop, Setup Section====================== //