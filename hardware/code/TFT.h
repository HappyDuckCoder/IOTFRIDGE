#ifndef TFT_H
#define TFT_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "DHT.h"
#include "gasSensor.h"
#include "Relay.h"

// Định nghĩa màu sắc
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_YELLOW    0xFFE0
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F
#define COLOR_ORANGE    0xFD20
#define COLOR_GRAY      0x8410
#define COLOR_LIGHT_GRAY 0xC618

// Kích thước màn hình (có thể điều chỉnh theo màn hình thực tế)
#define SCREEN_WIDTH    240
#define SCREEN_HEIGHT   320

enum DisplayPage
{
    PAGE_MAIN = 0,      // Trang chính hiển thị tất cả thông tin
    PAGE_SENSORS = 1,   // Trang chi tiết cảm biến
    PAGE_CONTROL = 2,   // Trang điều khiển
    PAGE_STATUS = 3     // Trang trạng thái hệ thống
};

struct TFTData
{
    DisplayPage currentPage;
    bool needsUpdate;
    unsigned long lastUpdate;
    unsigned long updateInterval;
    bool isConnected;
    bool isRecording;

    TFTData()
    {
        currentPage = PAGE_MAIN;
        needsUpdate = true;
        lastUpdate = 0;
        updateInterval = 1000; // Cập nhật mỗi giây
        isConnected = false;
        isRecording = false;
    }
};

class TFTDisplay
{
private:
    TFT_eSPI tft;
    TFTData data;

    // Vị trí hiển thị các thông tin
    const int HEADER_HEIGHT = 30;
    const int SECTION_HEIGHT = 60;
    const int MARGIN = 10;
    const int TEXT_SIZE_SMALL = 1;
    const int TEXT_SIZE_MEDIUM = 2;
    const int TEXT_SIZE_LARGE = 3;

public:
    TFTDisplay()
    {
        // Constructor
    }

    bool begin()
    {
        tft.init();
        tft.setRotation(0); // Có thể điều chỉnh hướng màn hình
        tft.fillScreen(COLOR_BLACK);

        // Hiển thị màn hình khởi động
        showBootScreen();
        delay(2000);

        data.needsUpdate = true;
        Serial.println("TFT Display khởi tạo thành công");
        return true;
    }

    void showBootScreen()
    {
        tft.fillScreen(COLOR_BLACK);
        tft.setTextColor(COLOR_WHITE);
        tft.setTextSize(TEXT_SIZE_LARGE);

        // Tiêu đề
        tft.setCursor(MARGIN, 50);
        tft.println("ESP32");
        tft.setCursor(MARGIN, 80);
        tft.println("SYSTEM");

        // Thông tin khởi động
        tft.setTextSize(TEXT_SIZE_SMALL);
        tft.setCursor(MARGIN, 150);
        tft.println("Khoi tao he thong...");

        // Thanh tiến trình giả lập
        drawProgressBar(MARGIN, 200, SCREEN_WIDTH - 2 * MARGIN, 20, 100, COLOR_GREEN);
    }

    void drawProgressBar(int x, int y, int width, int height, int progress, uint16_t color)
    {
        // Vẽ khung
        tft.drawRect(x, y, width, height, COLOR_WHITE);

        // Vẽ thanh tiến trình
        int fillWidth = (width - 2) * progress / 100;
        tft.fillRect(x + 1, y + 1, fillWidth, height - 2, color);
    }

    void nextPage()
    {
        data.currentPage = (DisplayPage)((data.currentPage + 1) % 4);
        data.needsUpdate = true;
    }

    void setRecordingState(bool recording)
    {
        if (data.isRecording != recording)
        {
            data.isRecording = recording;
            data.needsUpdate = true;
        }
    }

    void setConnectionState(bool connected)
    {
        if (data.isConnected != connected)
        {
            data.isConnected = connected;
            data.needsUpdate = true;
        }
    }

    void update(const DHTData& dhtData, const GasSensorData& gasData, const RelayData& relayData)
    {
        unsigned long currentTime = millis();

        if (data.needsUpdate || (currentTime - data.lastUpdate >= data.updateInterval))
        {
            switch (data.currentPage)
            {
            case PAGE_MAIN:
                drawMainPage(dhtData, gasData, relayData);
                break;
            case PAGE_SENSORS:
                drawSensorsPage(dhtData, gasData);
                break;
            case PAGE_CONTROL:
                drawControlPage(relayData);
                break;
            case PAGE_STATUS:
                drawStatusPage();
                break;
            }

            data.needsUpdate = false;
            data.lastUpdate = currentTime;
        }
    }

    void drawMainPage(const DHTData& dhtData, const GasSensorData& gasData, const RelayData& relayData)
    {
        tft.fillScreen(COLOR_BLACK);

        // Header
        drawHeader("MAIN SCREEN");

        int yPos = HEADER_HEIGHT + MARGIN;

        // Thông tin DHT11
        drawSensorSection("NHIET DO & DO AM", yPos, COLOR_CYAN);
        yPos += 20;

        if (dhtData.isValid)
        {
            tft.setTextColor(dhtData.temperature > 30 ? COLOR_RED : COLOR_GREEN);
            tft.setCursor(MARGIN, yPos);
            tft.printf("Nhiet do: %.1f°C", dhtData.temperature);
            yPos += 20;

            tft.setTextColor(dhtData.humidity > 70 ? COLOR_ORANGE : COLOR_GREEN);
            tft.setCursor(MARGIN, yPos);
            tft.printf("Do am: %.1f%%", dhtData.humidity);
        }
        else
        {
            tft.setTextColor(COLOR_RED);
            tft.setCursor(MARGIN, yPos);
            tft.println("DHT11: Loi doc du lieu");
        }

        yPos += 30;

        // Thông tin Gas
        drawSensorSection("CHAT LUONG KHONG KHI", yPos, COLOR_YELLOW);
        yPos += 20;

        if (gasData.isValid)
        {
            tft.setTextColor(gasData.isInDanger() ? COLOR_RED : COLOR_GREEN);
            tft.setCursor(MARGIN, yPos);
            tft.printf("MQ2: %.1f ppm", gasData.mq2Value);
            yPos += 20;

            tft.setCursor(MARGIN, yPos);
            tft.printf("MQ135: %.1f ppm", gasData.mq135Value);
        }
        else
        {
            tft.setTextColor(COLOR_RED);
            tft.setCursor(MARGIN, yPos);
            tft.println("Gas: Loi doc du lieu");
        }

        yPos += 30;

        // Trạng thái quạt
        drawSensorSection("TRANG THAI QUAT", yPos, COLOR_MAGENTA);
        yPos += 20;

        tft.setTextColor(relayData.isActive ? COLOR_GREEN : COLOR_GRAY);
        tft.setCursor(MARGIN, yPos);
        tft.printf("Che do: %s", getFanModeString(relayData.currentMode));

        // Status bar
        drawStatusBar();
    }

    void drawSensorsPage(const DHTData& dhtData, const GasSensorData& gasData)
    {
        tft.fillScreen(COLOR_BLACK);
        drawHeader("SENSORS DETAIL");

        int yPos = HEADER_HEIGHT + MARGIN;

        // Chi tiết DHT11
        tft.setTextColor(COLOR_CYAN);
        tft.setTextSize(TEXT_SIZE_MEDIUM);
        tft.setCursor(MARGIN, yPos);
        tft.println("DHT11 SENSOR");
        yPos += 30;

        tft.setTextSize(TEXT_SIZE_SMALL);
        if (dhtData.isValid)
        {
            tft.setTextColor(COLOR_WHITE);
            tft.setCursor(MARGIN, yPos);
            tft.printf("Temperature: %.2f°C", dhtData.temperature);
            yPos += 20;

            tft.setCursor(MARGIN, yPos);
            tft.printf("Humidity: %.2f%%", dhtData.humidity);
            yPos += 20;

            tft.setCursor(MARGIN, yPos);
            tft.printf("Last read: %lu ms", dhtData.lastReadTime);
        }
        else
        {
            tft.setTextColor(COLOR_RED);
            tft.setCursor(MARGIN, yPos);
            tft.println("ERROR: Cannot read data");
        }

        yPos += 40;

        // Chi tiết Gas sensors
        tft.setTextColor(COLOR_YELLOW);
        tft.setTextSize(TEXT_SIZE_MEDIUM);
        tft.setCursor(MARGIN, yPos);
        tft.println("GAS SENSORS");
        yPos += 30;

        tft.setTextSize(TEXT_SIZE_SMALL);
        if (gasData.isValid)
        {
            tft.setTextColor(COLOR_WHITE);
            tft.setCursor(MARGIN, yPos);
            tft.printf("MQ2: %.2f ppm", gasData.mq2Value);
            yPos += 20;

            tft.setCursor(MARGIN, yPos);
            tft.printf("MQ135: %.2f ppm", gasData.mq135Value);
            yPos += 20;

            tft.setCursor(MARGIN, yPos);
            tft.printf("Last read: %lu ms", gasData.lastReadTime);
            yPos += 20;

            if (gasData.isInDanger())
            {
                tft.setTextColor(COLOR_RED);
                tft.setCursor(MARGIN, yPos);
                tft.println("WARNING: High gas level!");
            }
        }
        else
        {
            tft.setTextColor(COLOR_RED);
            tft.setCursor(MARGIN, yPos);
            tft.println("ERROR: Cannot read data");
        }

        drawStatusBar();
    }

    void drawControlPage(const RelayData& relayData)
    {
        tft.fillScreen(COLOR_BLACK);
        drawHeader("CONTROL PANEL");

        int yPos = HEADER_HEIGHT + MARGIN;

        // Điều khiển quạt
        tft.setTextColor(COLOR_MAGENTA);
        tft.setTextSize(TEXT_SIZE_MEDIUM);
        tft.setCursor(MARGIN, yPos);
        tft.println("FAN CONTROL");
        yPos += 40;

        // Hiển thị chế độ hiện tại
        tft.setTextSize(TEXT_SIZE_LARGE);
        tft.setTextColor(relayData.isActive ? COLOR_GREEN : COLOR_GRAY);
        tft.setCursor(MARGIN, yPos);
        tft.println(getFanModeString(relayData.currentMode));
        yPos += 50;

        // Hiển thị các chế độ có sẵn
        tft.setTextSize(TEXT_SIZE_SMALL);
        tft.setTextColor(COLOR_WHITE);
        tft.setCursor(MARGIN, yPos);
        tft.println("Available modes:");
        yPos += 20;

        const char* modes[] = { "OFF", "LOW", "MEDIUM", "HIGH" };
        for (int i = 0; i < 4; i++)
        {
            tft.setTextColor(i == relayData.currentMode ? COLOR_GREEN : COLOR_GRAY);
            tft.setCursor(MARGIN + 20, yPos);
            tft.printf("%d. %s", i, modes[i]);
            yPos += 20;
        }

        drawStatusBar();
    }

    void drawStatusPage()
    {
        tft.fillScreen(COLOR_BLACK);
        drawHeader("SYSTEM STATUS");

        int yPos = HEADER_HEIGHT + MARGIN;

        // Trạng thái kết nối
        tft.setTextColor(COLOR_CYAN);
        tft.setTextSize(TEXT_SIZE_MEDIUM);
        tft.setCursor(MARGIN, yPos);
        tft.println("CONNECTION");
        yPos += 30;

        tft.setTextSize(TEXT_SIZE_SMALL);
        tft.setTextColor(data.isConnected ? COLOR_GREEN : COLOR_RED);
        tft.setCursor(MARGIN, yPos);
        tft.printf("WiFi: %s", data.isConnected ? "CONNECTED" : "DISCONNECTED");
        yPos += 30;

        // Trạng thái ghi âm
        tft.setTextColor(COLOR_ORANGE);
        tft.setTextSize(TEXT_SIZE_MEDIUM);
        tft.setCursor(MARGIN, yPos);
        tft.println("RECORDING");
        yPos += 30;

        tft.setTextSize(TEXT_SIZE_SMALL);
        tft.setTextColor(data.isRecording ? COLOR_RED : COLOR_GREEN);
        tft.setCursor(MARGIN, yPos);
        tft.printf("Status: %s", data.isRecording ? "RECORDING" : "IDLE");
        yPos += 30;

        // Thông tin hệ thống
        tft.setTextColor(COLOR_YELLOW);
        tft.setTextSize(TEXT_SIZE_MEDIUM);
        tft.setCursor(MARGIN, yPos);
        tft.println("SYSTEM INFO");
        yPos += 30;

        tft.setTextSize(TEXT_SIZE_SMALL);
        tft.setTextColor(COLOR_WHITE);
        tft.setCursor(MARGIN, yPos);
        tft.printf("Uptime: %lu ms", millis());
        yPos += 20;

        tft.setCursor(MARGIN, yPos);
        tft.printf("Free heap: %u bytes", ESP.getFreeHeap());

        drawStatusBar();
    }

    void drawHeader(const char* title)
    {
        tft.fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_BLUE);
        tft.setTextColor(COLOR_WHITE);
        tft.setTextSize(TEXT_SIZE_MEDIUM);
        tft.setCursor(MARGIN, 8);
        tft.println(title);
    }

    void drawSensorSection(const char* title, int yPos, uint16_t color)
    {
        tft.setTextColor(color);
        tft.setTextSize(TEXT_SIZE_SMALL);
        tft.setCursor(MARGIN, yPos);
        tft.println(title);
    }

    void drawStatusBar()
    {
        int barHeight = 20;
        int yPos = SCREEN_HEIGHT - barHeight;

        tft.fillRect(0, yPos, SCREEN_WIDTH, barHeight, COLOR_GRAY);

        tft.setTextColor(COLOR_WHITE);
        tft.setTextSize(TEXT_SIZE_SMALL);

        // WiFi status
        tft.setCursor(5, yPos + 5);
        tft.print(data.isConnected ? "WiFi" : "No WiFi");

        // Recording status
        if (data.isRecording)
        {
            tft.setTextColor(COLOR_RED);
            tft.setCursor(60, yPos + 5);
            tft.print("REC");
        }

        // Page indicator
        tft.setTextColor(COLOR_WHITE);
        tft.setCursor(SCREEN_WIDTH - 30, yPos + 5);
        tft.printf("%d/4", data.currentPage + 1);
    }

    const char* getFanModeString(FanMode mode) const
    {
        switch (mode)
        {
        case FAN_OFF: return "TAT";
        case FAN_LOW: return "CHAM";
        case FAN_MEDIUM: return "TB";
        case FAN_HIGH: return "NHANH";
        default: return "N/A";
        }
    }

    TFTData getData() const
    {
        return data;
    }

    void forceUpdate()
    {
        data.needsUpdate = true;
    }
};

#endif