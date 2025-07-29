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

    // tham số hỗ trợ thu
    long int i2s_read_len;
    long int sample_rate;
    int sample_bits;
    int channel;

public:
    I2SRecorder(INMP &mic, long int i2s_read_len, long int sample_rate, int sample_bits, int channel)
        : mic(mic), isRecording_(false), taskHandle(NULL), filename(nullptr), i2s_read_len(i2s_read_len), sample_rate(sample_rate),
          sample_bits(sample_bits), channel(channel) {}

    void start(const char *file_path)
    {
        if (isRecording_)
            return;

        filename = file_path;
        isRecording_ = true;

        file = SPIFFS.open(filename, FILE_WRITE);
        if (!file)
        {
            Serial.println("Không thể mở file để ghi.");
            isRecording_ = false;
            return;
        }

        xTaskCreate(taskEntry, "recordTask", 8192, this, 1, &taskHandle);
    }

    void stop()
    {
        Serial.println("Bắt đầu dừng ghi âm...");
        
        if (!isRecording_)
        {
            Serial.println("Không đang ghi âm, return");
            return;
        }

        Serial.println("Đang set isRecording_ = false");
        isRecording_ = false;

        if (taskHandle)
        {
            Serial.println("Chuẩn bị xóa task...");
            vTaskDelete(taskHandle);
            Serial.println("Đã xóa task");
            taskHandle = NULL;
        }

        if (file) 
        {
            Serial.println("Reset file handle");
            file = File();
        }
        else
        {
            Serial.println("Không có file handle để reset");
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
        const size_t buffer_size = i2s_read_len;
        char *buffer = (char *)malloc(buffer_size);

        while (isRecording_)
        {
            size_t bytesRead = mic.read(buffer, buffer_size);
            if (bytesRead > 0 && file)
            {
                file.write((const uint8_t *)buffer, bytesRead);
            }
        }

        free(buffer);
    }
};

#endif
