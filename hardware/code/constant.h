#ifndef CONSTANT_H
#define CONSTANT_H

// ============================================
// SENSOR PINS
// ============================================
#define MQ2_PIN 34       // Cảm biến khí gas MQ2
#define MQ135_PIN 35     // Cảm biến chất lượng không khí MQ135
#define DHT_PIN 5        // Cảm biến nhiệt độ và độ ẩm DHT11
#define INMP_WS 21       // Word Select (LRC)
#define INMP_SD 23       // Serial Data (DIN)
#define INMP_SCK 22      // Serial Clock (BCLK)
#define RECORD_BUTTON 19 // Nút ghi âm
#define RELAY_PIN 6      // Pin relay điều khiển quạt
#define PWM_PIN 7        // Pin PWM điều khiển tốc độ quạt
// #define TFT_MISO 19
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS   15
// #define TFT_DC   2
// #define TFT_RST  4
// #define TFT_BL   32
#define DOOR_BUTTON_PIN 8   // Nút cảm biến cửa
#define LOADCELL_DOUT_PIN 9 // Chân DOUT nối tốc dụng load cell
#define LOADCELL_SCK_PIN 10 // Chân SCK nối tốc dụng load cell

#define MQ2_DANGER_THRESHOLD 300.0   // Ngưỡng nguy hiểm cho MQ2 (ppm)
#define MQ135_DANGER_THRESHOLD 200.0 // Ngưỡng nguy hiểm cho MQ135 (ppm)
#define TEMP_HIGH_THRESHOLD 30.0     // Ngưỡng nhiệt độ cao (°C)
#define HUMIDITY_HIGH_THRESHOLD 70.0 // Ngưỡng độ ẩm cao (%)

// PWM cho các mức tốc độ quạt
#define FAN_SPEED_OFF 0      // Tắt quạt
#define FAN_SPEED_LOW 85     // Tốc độ thấp (33%)
#define FAN_SPEED_MEDIUM 170 // Tốc độ trung bình (66%)
#define FAN_SPEED_HIGH 255   // Tốc độ cao (100%)

#define MAX_DOOR_OPEN_TIME 1000 * 60 * 5 // Thời gian cửa bộ bằng 5 phút

const char *audioFileName = "/recording.wav";

#endif