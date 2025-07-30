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
    SemaphoreHandle_t stopSemaphore;  // Semaphore để đồng bộ hóa việc dừng

    // tham số hỗ trợ thu
    long int i2s_read_len;
    long int sample_rate;
    int sample_bits;
    int channel;

public:
    I2SRecorder(INMP &mic, long int i2s_read_len, long int sample_rate, int sample_bits, int channel)
        : mic(mic), isRecording_(false), taskHandle(NULL), filename(nullptr), stopSemaphore(NULL),
          i2s_read_len(i2s_read_len), sample_rate(sample_rate), sample_bits(sample_bits), channel(channel) 
    {
        // Tạo semaphore để đồng bộ hóa việc dừng
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

        // Reset semaphore trước khi bắt đầu task mới
        xSemaphoreTake(stopSemaphore, 0);
        
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
            Serial.println("Đợi task kết thúc an toàn...");
            // Đợi task tự kết thúc thay vì xóa đột ngột
            if (xSemaphoreTake(stopSemaphore, pdMS_TO_TICKS(2000)) == pdTRUE) {
                Serial.println("Task đã kết thúc an toàn");
            } else {
                Serial.println("Timeout khi đợi task kết thúc");
            }
            taskHandle = NULL;
        }

        // Đóng file một cách an toàn
        if (file) 
        {
            Serial.println("Đóng file...");
            file.flush();  // Đảm bảo tất cả dữ liệu được ghi xuống
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
        const size_t buffer_size = i2s_read_len;
        char *buffer = (char *)malloc(buffer_size);
        
        if (!buffer) {
            Serial.println("Không thể cấp phát bộ nhớ cho buffer");
            isRecording_ = false;
            xSemaphoreGive(stopSemaphore);  // Báo hiệu task đã kết thúc
            vTaskDelete(NULL);
            return;
        }

        Serial.println("Task ghi âm đã bắt đầu");

        while (isRecording_)
        {
            size_t bytesRead = mic.read(buffer, buffer_size);
            if (bytesRead > 0 && file)
            {
                size_t written = file.write((const uint8_t *)buffer, bytesRead);
                if (written != bytesRead) {
                    Serial.println("Lỗi ghi file, dừng ghi âm");
                    break;
                }
            }
            
            // Cho CPU nghỉ một chút
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        Serial.println("Task ghi âm đang kết thúc...");
        free(buffer);
        
        // Báo hiệu rằng task đã kết thúc
        xSemaphoreGive(stopSemaphore);
        
        Serial.println("Task ghi âm đã kết thúc");
        vTaskDelete(NULL);  // Tự xóa task
    }
};

#endif