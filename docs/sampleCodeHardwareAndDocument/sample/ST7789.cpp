#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h> // Thay đổi sang ST7789
#include <SPI.h>

// Định nghĩa chân cho ESP32 NodeMCU 32S
#define TFT_CS 5  // GPIO 5 (bên phải board)
#define TFT_RST 4 // GPIO 4 (bên phải board)
#define TFT_DC 2  // GPIO 2 (bên phải board)

// Khởi tạo màn hình ST7789
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Bắt đầu test ST7789...");

    Serial.println("1. Khởi tạo SPI cho NodeMCU 32S...");
    // NodeMCU 32S: SCK=GPIO22, MISO=không dùng, MOSI=GPIO23
    SPI.begin(22, -1, 23, -1);

    Serial.println("2. Khởi tạo màn hình ST7789...");

    // Thử nhiều kích thước khác nhau
    Serial.println("Thử 240x240...");
    tft.init(240, 240);
    tft.fillScreen(ST77XX_RED);
    delay(2000);

    Serial.println("Thử 135x240...");
    tft.init(135, 240);
    tft.fillScreen(ST77XX_GREEN);
    delay(2000);

    Serial.println("Thử 240x320...");
    tft.init(240, 320);
    tft.fillScreen(ST77XX_BLUE);
    delay(2000);

    Serial.println("Thử 128x160...");
    tft.init(128, 160);
    tft.fillScreen(ST77XX_YELLOW);
    delay(2000);

    Serial.println("3. Test màn hình cơ bản...");

    // Test với kích thước cuối cùng được init
    tft.fillScreen(ST77XX_WHITE);
    delay(1000);
    tft.fillScreen(ST77XX_BLACK);
    delay(1000);

    // Test rotation
    Serial.println("Test xoay màn hình...");
    for (int r = 0; r < 4; r++)
    {
        tft.setRotation(r);
        tft.fillScreen(ST77XX_RED + r * 1000);
        tft.setCursor(10, 10);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(2);
        tft.print("Rotation: ");
        tft.println(r);
        delay(2000);
    }

    // Reset về rotation 0
    tft.setRotation(0);

    // Hiển thị text
    Serial.println("Test text...");
    tft.setCursor(20, 50);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(3);
    tft.println("ST7789");

    tft.setCursor(20, 100);
    tft.setTextColor(ST77XX_CYAN);
    tft.setTextSize(2);
    tft.println("ESP32 OK!");

    // Vẽ hình chữ nhật
    tft.drawRect(10, 150, 100, 60, ST77XX_RED);
    tft.fillRect(15, 155, 90, 50, ST77XX_MAGENTA);

    // Vẽ hình tròn
    tft.drawCircle(180, 180, 30, ST77XX_GREEN);
    tft.fillCircle(180, 180, 25, ST77XX_YELLOW);

    Serial.println("4. Hoàn thành!");
}

void loop()
{
    Serial.println("Loop đang chạy...");
    delay(3000);

    // Nhấp nháy viền
    tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_WHITE);
    delay(500);
    tft.drawRect(0, 0, tft.width(), tft.height(), ST77XX_BLACK);
    delay(500);
}