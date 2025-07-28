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
#define TFT_CS_PIN 5
#define TFT_DC_PIN 2
#define TFT_RST_PIN 4
#define TFT_SCLK_PIN 22
#define TFT_MOSI_PIN 23
#define TFT_SCREEN_WIDTH 240
#define TFT_SCREEN_HEIGHT 320

struct FridgeData
{
  float temp;
  float humi;
  bool is_rotted_food;
  int total_food;
  int last_open;

  FridgeData(float temp, float humi, bool is_rotted_food, int total_food, int last_open)
      : temp(temp), humi(humi), is_rotted_food(is_rotted_food), total_food(total_food), last_open(last_open) {}
};

#endif