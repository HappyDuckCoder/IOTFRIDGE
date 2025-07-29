#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Khởi tạo đối tượng màn hình với I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
    Serial.begin(115200);

    // Khởi tạo màn hình OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // 0x3C là địa chỉ I2C mặc định
        Serial.println(F("SSD1306 không tìm thấy"));
        for (;;)
            ; // Dừng tại đây nếu không kết nối được
    }

    display.clearDisplay();              // Xóa màn hình
    display.setTextSize(1);              // Kích thước chữ
    display.setTextColor(SSD1306_WHITE); // Màu chữ
    display.setCursor(0, 0);             // Vị trí bắt đầu
    display.println("Hello, World!");    // Nội dung
    display.display();                   // Cập nhật hiển thị
}

void loop()
{
    // không làm gì trong loop
}
