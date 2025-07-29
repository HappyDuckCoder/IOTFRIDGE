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
        if (ESP.getFreeHeap() < 30000) {
            Serial.println("KHÔNG ĐỦ MEMORY ĐỂ KHỞI TẠO I2S!");
            return false;
        }

        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = 16000,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = 0,
            .dma_buf_count = 8,
            .dma_buf_len = 512,
            .use_apll = true
        };

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
            .data_in_num = data_in_pin
        };

        err = i2s_set_pin(I2S_NUM_0, &pin_config);
        if (err != ESP_OK)
        {
            Serial.println("Không cấu hình chân I2S");
            return false;
        }

        return true;
    }

    size_t read(char *buffer, size_t len)
    {
        size_t bytesRead = 0;
        esp_err_t result = i2s_read(I2S_NUM_0, buffer, len, &bytesRead, 100 / portTICK_RATE_MS);
        if (result != ESP_OK) {
            Serial.printf("I2S read error: %d\n", result);
            return 0;
        }
        return bytesRead;
    }
};

#endif
