#ifndef TFT_H
#define TFT_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// Những thông tin muốn hiện
// Nhiệt độ, độ ẩm, có đồ ăn bị hư, tổng số lượng món ăn, lần cuối đóng mở cửa tủ

class TFTDisplay
{
private:
    Adafruit_ST7789 tft;
    int TFT_SCLK, TFT_MOSI;
    int width, height;

public:
    TFTDisplay(int cs, int dc, int rst, int sclk, int mosi, int w, int h)
        : tft(cs, dc, rst), TFT_SCLK(sclk), TFT_MOSI(mosi), width(w), height(h) {}

    bool begin()
    {
        SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1);
        tft.init(width, height);
        tft.fillScreen(ST77XX_BLACK);
        return true;
    }

    void showHello()
    {
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(30, 60);
        tft.setTextColor(ST77XX_YELLOW);
        tft.setTextSize(3);
        tft.println("Hello");
    }

    void drawText(String type, String text, int x, int y, int color, int size = 2)
    {
        if (type == "title")
            size = 3;
        else if (type == "sub_title")
            size = 2;
        else if (type == "body")
            size = 1;

        tft.setCursor(x, y);
        tft.setTextColor(color);
        tft.setTextSize(size);
        tft.print(text);
    }

    void drawBox(int w, int h, int color, int x, int y)
    {
        tft.fillRect(x, y, w, h, color);
    }

    void drawRectangle(int w, int h, int color, int x, int y)
    {
        tft.drawRect(x, y, w, h, color);
    }

    void drawBackground()
    {
        tft.fillScreen(ST77XX_BLACK);
        drawRectangle(width - 20, height - 100, ST77XX_WHITE, 10, 40);
        drawText("title", "IOT Fridge", 30, 70, ST77XX_CYAN);
    }

    void showMain(float temp, float humi, bool is_rotted_food, int total_food, int last_open)
    {
        drawBackground();

        const char *labels[] = {
            "Temp:",
            "Humidity:",
            "Food count:",
            "Rotten food:",
            "Last open:"};

        String values[] = {
            String(temp) + " C",
            String(humi) + " %",
            String(total_food),
            is_rotted_food ? "Yes" : "No",
            String(last_open) + "h"};

        uint16_t colors[] = {
            ST77XX_WHITE,
            ST77XX_WHITE,
            ST77XX_WHITE,
            is_rotted_food ? ST77XX_RED : ST77XX_GREEN,
            ST77XX_WHITE};

        int startY = 100;
        int startX_sub_title = 20;
        int startX_body = 190;
        for (int i = 0; i < 5; ++i)
        {
            drawText("sub_title", labels[i], startX_sub_title, startY + i * 30, ST77XX_YELLOW);
            drawText("body", values[i], startX_body, startY + i * 30 + 5, colors[i]);
        }
    }
};

#endif
