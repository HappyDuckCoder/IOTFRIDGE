#ifndef I2S_RECORDER_H
#define I2S_RECORDER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include "INMP.h"

class I2SRecorder
{
private:
    INMP &mic;
    File file;
    TaskHandle_t taskHandle;
    bool isRecording_;
    const char *filename;
    SemaphoreHandle_t stopSemaphore;

    // thông số cấu hình
    long int i2s_read_len;  // số sample sẽ đọc mỗi vòng
    long int sample_rate;
    int sample_bits;
    int channel;

public:
    I2SRecorder(INMP &mic, long int i2s_read_len, long int sample_rate, int sample_bits, int channel)
        : mic(mic), isRecording_(false), taskHandle(NULL), filename(nullptr), stopSemaphore(NULL),
          i2s_read_len(i2s_read_len), sample_rate(sample_rate), sample_bits(sample_bits), channel(channel)
    {
        stopSemaphore = xSemaphoreCreateBinary();
    }

    ~I2SRecorder()
    {
        if (stopSemaphore) {
            vSemaphoreDelete(stopSemaphore);
        }
    }

    void start(const char *file_path)
    {
        if (isRecording_) return;

        filename = file_path;
        isRecording_ = true;

        file = SPIFFS.open(filename, FILE_WRITE);
        if (!file) {
            Serial.println("Không thể mở file để ghi.");
            isRecording_ = false;
            return;
        }

        xSemaphoreTake(stopSemaphore, 0);  // reset trước khi tạo task
        xTaskCreate(taskEntry, "recordTask", 8192, this, 1, &taskHandle);
    }

    void stop()
    {
        Serial.println("Bắt đầu dừng ghi âm...");

        if (!isRecording_) {
            Serial.println("Không đang ghi âm, return");
            return;
        }

        isRecording_ = false;

        if (taskHandle) {
            Serial.println("Đợi task kết thúc an toàn...");
            if (xSemaphoreTake(stopSemaphore, pdMS_TO_TICKS(2000)) == pdTRUE) {
                Serial.println("Task đã kết thúc an toàn");
            } else {
                Serial.println("Timeout khi đợi task kết thúc");
            }
            taskHandle = NULL;
        }

        if (file) {
            Serial.println("Đóng file...");
            file.flush();
            file.close();
            Serial.println("Đã đóng file");
        }

        Serial.println("Dừng ghi âm hoàn tất!");
    }

    bool isRecording() const
    {
        return isRecording_;
    }

private:
    static void taskEntry(void *param)
    {
        static_cast<I2SRecorder *>(param)->taskLoop();
    }

    void taskLoop()
    {
        const size_t sample_count = i2s_read_len;
        int16_t *buffer = (int16_t *)malloc(sample_count * sizeof(int16_t));

        if (!buffer) {
            Serial.println("Không thể cấp phát bộ nhớ cho buffer");
            isRecording_ = false;
            xSemaphoreGive(stopSemaphore);
            vTaskDelete(NULL);
            return;
        }

        Serial.println("Task ghi âm đã bắt đầu");

        while (isRecording_)
        {
            size_t samplesRead = mic.read(buffer, sample_count);
            if (samplesRead > 0 && file)
            {
                size_t bytesToWrite = samplesRead * sizeof(int16_t);
                size_t written = file.write((const uint8_t *)buffer, bytesToWrite);
                if (written != bytesToWrite) {
                    Serial.println("Lỗi ghi file, dừng ghi âm");
                    break;
                }
            }

            vTaskDelay(pdMS_TO_TICKS(1));
        }

        Serial.println("Task ghi âm đang kết thúc...");
        free(buffer);
        xSemaphoreGive(stopSemaphore);
        Serial.println("Task ghi âm đã kết thúc");
        vTaskDelete(NULL);
    }
};

#endif
