#ifndef INMP_H
#define INMP_H

#include <driver/i2s.h>
#include <SPIFFS.h>
#include <vector>

class INMP
{
private:
    // Cấu hình I2S
    i2s_port_t i2sPort;
    int sampleRate;
    int bitsPerSample;
    int channels;
    int readBufferSize;
    int maxRecordTime;

    // Chân kết nối
    int pinWS;
    int pinSD;
    int pinSCK;

    // Trạng thái
    bool recording;
    bool initialized;
    File audioFile;
    TaskHandle_t recordingTaskHandle;

    // Callback function
    typedef void (*RecordingCallback)(bool isRecording, int progress);
    RecordingCallback callback;

    // Tính kích thước file tối đa
    int getMaxFileSize()
    {
        return channels * sampleRate * bitsPerSample / 8 * maxRecordTime;
    }

    // Task ghi âm
    static void recordingTask(void *pvParameters)
    {
        INMP *inmp = (INMP *)pvParameters;
        inmp->recordAudio();
    }

    void recordAudio()
    {
        int readLen = readBufferSize;
        int totalWritten = 0;
        int maxSize = getMaxFileSize();
        size_t bytesRead;

        char *readBuffer = (char *)calloc(readLen, sizeof(char));
        uint8_t *writeBuffer = (uint8_t *)calloc(readLen, sizeof(char));

        if (!readBuffer || !writeBuffer)
        {
            Serial.println("Không đủ bộ nhớ để ghi âm!");
            if (readBuffer)
                free(readBuffer);
            if (writeBuffer)
                free(writeBuffer);
            recording = false;
            if (callback)
                callback(false, 0);
            vTaskDelete(NULL);
            return;
        }

        // Xóa buffer cũ
        i2s_read(i2sPort, (void *)readBuffer, readLen, &bytesRead, portMAX_DELAY);
        i2s_read(i2sPort, (void *)readBuffer, readLen, &bytesRead, portMAX_DELAY);

        Serial.println("*** Đang ghi âm ***");
        if (callback)
            callback(true, 0);

        int lastProgress = 0;
        while (recording && totalWritten < maxSize)
        {
            esp_err_t result = i2s_read(i2sPort, (void *)readBuffer, readLen, &bytesRead, 100);

            if (result == ESP_OK && bytesRead > 0)
            {
                processAudioData(writeBuffer, (uint8_t *)readBuffer, readLen);
                audioFile.write((const byte *)writeBuffer, readLen);
                totalWritten += readLen;

                // Tính và gọi callback tiến trình
                int progress = totalWritten * 100 / maxSize;
                if (progress != lastProgress && progress % 10 == 0)
                {
                    Serial.printf("Tiến trình ghi âm: %d%%\n", progress);
                    if (callback)
                        callback(true, progress);
                    lastProgress = progress;
                }
            }

            vTaskDelay(1);
        }

        free(readBuffer);
        free(writeBuffer);

        if (recording)
        {
            Serial.println("*** Hoàn thành ghi âm (đạt thời gian tối đa) ***");
            recording = false;
            audioFile.close();
            if (callback)
                callback(false, 100);
        }

        recordingTaskHandle = NULL;
        vTaskDelete(NULL);
    }

    void processAudioData(uint8_t *dest, uint8_t *src, uint32_t len)
    {
        uint32_t j = 0;
        uint32_t dacValue = 0;

        for (int i = 0; i < len; i += 2)
        {
            dacValue = ((((uint16_t)(src[i + 1] & 0xf) << 8) | ((src[i + 0]))));
            dest[j++] = 0;
            dest[j++] = dacValue * 256 / 2048;
        }
    }

    void createWAVHeader(byte *header, int wavSize)
    {
        // RIFF Header
        header[0] = 'R';
        header[1] = 'I';
        header[2] = 'F';
        header[3] = 'F';

        // File Size
        unsigned int fileSize = wavSize + 44 - 8;
        header[4] = (byte)(fileSize & 0xFF);
        header[5] = (byte)((fileSize >> 8) & 0xFF);
        header[6] = (byte)((fileSize >> 16) & 0xFF);
        header[7] = (byte)((fileSize >> 24) & 0xFF);

        // WAVE Format
        header[8] = 'W';
        header[9] = 'A';
        header[10] = 'V';
        header[11] = 'E';

        // Format Chunk
        header[12] = 'f';
        header[13] = 'm';
        header[14] = 't';
        header[15] = ' ';
        header[16] = 0x10;
        header[17] = 0x00;
        header[18] = 0x00;
        header[19] = 0x00;

        // Audio Format (PCM)
        header[20] = 0x01;
        header[21] = 0x00;

        // Channels
        header[22] = 0x01;
        header[23] = 0x00;

        // Sample Rate
        header[24] = 0x80;
        header[25] = 0x3E;
        header[26] = 0x00;
        header[27] = 0x00;

        // Byte Rate
        header[28] = 0x00;
        header[29] = 0x7D;
        header[30] = 0x01;
        header[31] = 0x00;

        // Block Align
        header[32] = 0x02;
        header[33] = 0x00;

        // Bits per Sample
        header[34] = 0x10;
        header[35] = 0x00;

        // Data Chunk
        header[36] = 'd';
        header[37] = 'a';
        header[38] = 't';
        header[39] = 'a';

        // Data Size
        header[40] = (byte)(wavSize & 0xFF);
        header[41] = (byte)((wavSize >> 8) & 0xFF);
        header[42] = (byte)((wavSize >> 16) & 0xFF);
        header[43] = (byte)((wavSize >> 24) & 0xFF);
    }

public:
    INMP(int ws = 21, int sd = 23, int sck = 22)
    {
        i2sPort = I2S_NUM_0;
        sampleRate = 16000;
        bitsPerSample = 16;
        channels = 1;
        readBufferSize = 16 * 1024;
        maxRecordTime = 10;

        pinWS = ws;
        pinSD = sd;
        pinSCK = sck;

        recording = false;
        initialized = false;
        recordingTaskHandle = NULL;
        callback = NULL;
    }

    bool begin()
    {
        if (initialized)
            return true;

        // Cấu hình I2S
        i2s_config_t i2s_config = {
            .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = sampleRate,
            .bits_per_sample = i2s_bits_per_sample_t(bitsPerSample),
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
            .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = 0,
            .dma_buf_count = 64,
            .dma_buf_len = 1024,
            .use_apll = 1};

        if (i2s_driver_install(i2sPort, &i2s_config, 0, NULL) != ESP_OK)
        {
            Serial.println("Lỗi cài đặt I2S driver");
            return false;
        }

        // Cấu hình chân
        const i2s_pin_config_t pin_config = {
            .bck_io_num = pinSCK,
            .ws_io_num = pinWS,
            .data_out_num = -1,
            .data_in_num = pinSD};

        if (i2s_set_pin(i2sPort, &pin_config) != ESP_OK)
        {
            Serial.println("Lỗi cấu hình chân I2S");
            return false;
        }

        initialized = true;
        Serial.println("I2S khởi tạo thành công");
        return true;
    }

    void setRecordingCallback(RecordingCallback cb)
    {
        callback = cb;
    }

    bool startRecording(const char *filename)
    {
        if (!initialized || recording)
            return false;

        SPIFFS.remove(filename);
        audioFile = SPIFFS.open(filename, FILE_WRITE);
        if (!audioFile)
        {
            Serial.println("Không thể tạo file ghi âm!");
            return false;
        }

        // Tạo và ghi WAV header
        byte header[44];
        createWAVHeader(header, getMaxFileSize());
        audioFile.write(header, 44);

        Serial.println("=== Bắt đầu ghi âm ===");
        recording = true;

        xTaskCreate(recordingTask, "recording_task", 4096, this, 1, &recordingTaskHandle);
        return true;
    }

    bool stopRecording()
    {
        if (!recording)
            return false;

        Serial.println("=== Dừng ghi âm ===");
        recording = false;

        if (recordingTaskHandle)
        {
            vTaskDelete(recordingTaskHandle);
            recordingTaskHandle = NULL;
        }

        if (audioFile)
        {
            audioFile.close();
        }

        if (callback)
            callback(false, 100);
        return true;
    }

    bool isRecording()
    {
        return recording;
    }

    void setSampleRate(int rate)
    {
        if (!recording)
            sampleRate = rate;
    }

    void setRecordTime(int seconds)
    {
        if (!recording)
            maxRecordTime = seconds;
    }

    void listFiles()
    {
        Serial.println(F("\r\nDanh sách files trong SPIFFS:"));
        Serial.println(F("================================================="));
        Serial.println(F("  Tên file                              Kích thước"));
        Serial.println(F("================================================="));

        fs::File root = SPIFFS.open("/");
        if (!root || !root.isDirectory())
        {
            Serial.println(F("Không thể mở thư mục SPIFFS"));
            return;
        }

        fs::File file = root.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                String fileName = file.name();
                Serial.print("  " + fileName);

                int spaces = 33 - fileName.length();
                if (spaces < 1)
                    spaces = 1;
                while (spaces--)
                    Serial.print(" ");

                String fileSize = String(file.size());
                spaces = 10 - fileSize.length();
                if (spaces < 1)
                    spaces = 1;
                while (spaces--)
                    Serial.print(" ");
                Serial.println(fileSize + " bytes");
            }
            file = root.openNextFile();
        }

        Serial.println(F("================================================="));
    }

    void clearAllFiles()
    {
        Serial.println("=== Xóa các file cũ trong SPIFFS ===");

        fs::File root = SPIFFS.open("/");
        if (!root || !root.isDirectory())
        {
            Serial.println("Không thể mở thư mục SPIFFS");
            return;
        }

        std::vector<String> filesToDelete;
        fs::File file = root.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                filesToDelete.push_back(file.name());
            }
            file = root.openNextFile();
        }

        int deletedCount = 0;
        for (String fileName : filesToDelete)
        {
            if (SPIFFS.remove(fileName))
            {
                Serial.println("✓ Đã xóa: " + fileName);
                deletedCount++;
            }
            else
            {
                Serial.println("✗ Không thể xóa: " + fileName);
            }
        }

        Serial.printf("Đã xóa %d/%d file(s)\n", deletedCount, filesToDelete.size());

        size_t totalBytes = SPIFFS.totalBytes();
        size_t usedBytes = SPIFFS.usedBytes();
        Serial.printf("Dung lượng SPIFFS: %d bytes tổng, %d bytes đã dùng, %d bytes trống\n",
                      totalBytes, usedBytes, totalBytes - usedBytes);
    }
};

#endif