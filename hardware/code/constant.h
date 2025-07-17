#ifndef CONSTANT_H
#define CONSTANT_H

// ============================================
// SENSOR PINS
// ============================================
#define MQ2_PIN 34              // Cảm biến khí gas MQ2
#define MQ135_PIN 35            // Cảm biến chất lượng không khí MQ135
#define DHT_PIN 5               // Cảm biến nhiệt độ và độ ẩm DHT11

// ============================================
// AUDIO RECORDING PINS (I2S INMP441)
// ============================================
#define INMP_WS 21              // Word Select (LRC)
#define INMP_SD 23              // Serial Data (DIN)
#define INMP_SCK 22             // Serial Clock (BCLK)

// ============================================
// CONTROL BUTTONS
// ============================================
#define RECORD_BUTTON 19        // Nút ghi âm
#define FAN_BUTTON 4            // Nút điều khiển quạt
#define DISPLAY_BUTTON 0        // Nút chuyển trang màn hình (thường dùng BOOT button)

// ============================================
// RELAY AND FAN CONTROL
// ============================================
#define RELAY_PIN 6             // Pin relay điều khiển quạt
#define PWM_PIN 7               // Pin PWM điều khiển tốc độ quạt

// ============================================
// TFT DISPLAY PINS (TFT_eSPI sử dụng SPI)
// ============================================
// Pins được định nghĩa trong User_Setup.h của TFT_eSPI library
// Thường sử dụng:
// #define TFT_MISO 19
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS   15
// #define TFT_DC   2
// #define TFT_RST  4
// #define TFT_BL   32  // Backlight

// ============================================
// SYSTEM CONSTANTS
// ============================================
#define SERIAL_BAUDRATE 115200  // Tốc độ Serial
#define SENSOR_READ_INTERVAL 2000  // Đọc cảm biến mỗi 2 giây
#define DISPLAY_UPDATE_INTERVAL 1000  // Cập nhật màn hình mỗi 1 giây
#define BUTTON_DEBOUNCE_TIME 50  // Thời gian debounce cho button (ms)

// ============================================
// SENSOR THRESHOLDS
// ============================================
#define MQ2_DANGER_THRESHOLD 300.0    // Ngưỡng nguy hiểm cho MQ2 (ppm)
#define MQ135_DANGER_THRESHOLD 200.0  // Ngưỡng nguy hiểm cho MQ135 (ppm)
#define TEMP_HIGH_THRESHOLD 30.0      // Ngưỡng nhiệt độ cao (°C)
#define HUMIDITY_HIGH_THRESHOLD 70.0  // Ngưỡng độ ẩm cao (%)

// ============================================
// FAN CONTROL CONSTANTS
// ============================================
#define FAN_PWM_FREQUENCY 1000  // Tần số PWM cho quạt (Hz)
#define FAN_PWM_RESOLUTION 8    // Độ phân giải PWM (8-bit = 0-255)
#define FAN_PWM_CHANNEL 0       // Kênh PWM

// PWM duty cycle cho các mức tốc độ quạt
#define FAN_SPEED_OFF 0         // Tắt quạt
#define FAN_SPEED_LOW 85        // Tốc độ thấp (33%)
#define FAN_SPEED_MEDIUM 170    // Tốc độ trung bình (66%)
#define FAN_SPEED_HIGH 255      // Tốc độ cao (100%)

// ============================================
// AUDIO RECORDING CONSTANTS
// ============================================
#define SAMPLE_RATE 16000       // Tần số lấy mẫu âm thanh
#define SAMPLE_BITS 16          // Độ phân giải bit
#define RECORD_TIME 10          // Thời gian ghi âm (giây)
#define I2S_PORT I2S_NUM_0      // Port I2S sử dụng

// ============================================
// WIFI CONSTANTS
// ============================================
#define WIFI_CONNECT_TIMEOUT 10000  // Timeout kết nối WiFi (ms)
#define WIFI_RETRY_DELAY 5000       // Thời gian chờ retry WiFi (ms)

// ============================================
// SYSTEM STATES
// ============================================
#define SYSTEM_IDLE 0
#define SYSTEM_RECORDING 1
#define SYSTEM_PROCESSING 2
#define SYSTEM_ERROR 3

// ============================================
// ERROR CODES
// ============================================
#define ERROR_NONE 0
#define ERROR_SENSOR_DHT 1
#define ERROR_SENSOR_GAS 2
#define ERROR_WIFI_CONNECT 3
#define ERROR_AUDIO_INIT 4
#define ERROR_DISPLAY_INIT 5

// ============================================
// TIMING CONSTANTS
// ============================================
#define WATCHDOG_TIMEOUT 30000  // Watchdog timeout (ms)
#define LOOP_DELAY 10           // Delay chính trong loop (ms)
#define SENSOR_WARMUP_TIME 5000 // Thời gian khởi động cảm biến (ms)

#endif