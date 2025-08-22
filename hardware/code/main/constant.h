#ifndef CONSTANT_H
#define CONSTANT_H

// Button
#define BUTTON_MIC_PIN 16
#define BUTTON_FAN_PIN 17
#define BUTTON_DOOR_PIN 18

// DHT
#define DHT_PIN 19
#define DHT_TYPE DHT11

// MQ
#define MQ2_PIN 32
#define MQ135_PIN 33

// relay
#define RELAY_PIN 21

// TFT
#define TFT_CS_PIN 25
#define TFT_DC_PIN 2
#define TFT_RST_PIN 26
#define TFT_SCLK_PIN 22
#define TFT_MOSI_PIN 23
#define TFT_SCREEN_WIDTH 240 //135
#define TFT_SCREEN_HEIGHT 320 //240

// INMP
#define INMP_BCLK_PIN 12
#define INMP_WS_PIN 13
#define INMP_DATA_PIN 14

// I2S 
#define I2S_READ_LEN (2 * 1024)
#define SAMPLE_RATE 16000
#define SAMPLE_BITS 16
#define CHANNEL_NUM 1

// HX711
#define HX711_SCK_PIN 4
#define HX711_DOUT_PIN 5
#define WEIGHT_CHANGE_THRESHOLD 10.0f  // 50 g
#define DOOR_ALERT_TIMEOUT 10000UL     // 30 s cửa mở thì cảnh báo
#define DOOR_JUST_CLOSED_TIME 3000UL  // < 20 s kể từ lúc đóng => "just closed"
#define PUSH_MIN_INTERVAL_MS 60000UL   // tối thiểu 1 phút giữa 2 lần đẩy

struct FridgeData
{
  float temp;
  float humi;
  bool is_rotted_food;
  int total_food;
  int last_open;
  bool is_saving_mode; 

  FridgeData(float temp, float humi, bool is_rotted_food, int total_food, int last_open, bool is_saving_mode)
      : temp(temp), humi(humi), is_rotted_food(is_rotted_food), total_food(total_food), last_open(last_open), is_saving_mode(is_saving_mode) {}
};

#endif