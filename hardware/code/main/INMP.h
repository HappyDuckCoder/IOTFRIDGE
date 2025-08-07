#ifndef INMP_H
#define INMP_H

#include <Arduino.h>
#include <driver/i2s.h>

class INMP
{
private:
    int bclk_pin;
    int ws_pin;
    int data_in_pin;

public:
    INMP(int bclk, int ws, int data_in) : bclk_pin(bclk), ws_pin(ws), data_in_pin(data_in) {}

    bool begin()
    {
        if (ESP.getFreeHeap() < 30000)
        {
            Serial.println("KHÔNG ĐỦ MEMORY ĐỂ KHỞI TẠO I2S!");
            return false;
        }

        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = 16000,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 4,
            .dma_buf_len = 1024,
            .use_apll = false,
            .tx_desc_auto_clear = true,
            .fixed_mclk = 0};

        esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
        if (err != ESP_OK)
        {
            Serial.println("Không cài driver I2S");
            return false;
        }

        const i2s_pin_config_t pin_config = {
            .bck_io_num = bclk_pin,
            .ws_io_num = ws_pin,
            .data_out_num = -1,
            .data_in_num = data_in_pin};

        err = i2s_set_pin(I2S_NUM_0, &pin_config);
        if (err != ESP_OK)
        {
            Serial.println("Không cấu hình chân I2S");
            return false;
        }

        return true;
    }

    // Ghi đúng kiểu 16-bit (signed short)
    size_t read(int16_t *buffer, size_t samples)
    {
        size_t bytesRead = 0;
        esp_err_t result = i2s_read(I2S_NUM_0, buffer, samples * sizeof(int16_t), &bytesRead, portMAX_DELAY);

        if (result == ESP_OK && bytesRead > 0)
        {
            return bytesRead / sizeof(int16_t); // Trả về số samples đã đọc
        }
        return 0;
    }
};

#endif
