#include <driver/i2s.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define I2S_WS 21
#define I2S_SD 23
#define I2S_SCK 22

#define BUTTON 19

#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (16000)
#define I2S_SAMPLE_BITS (16)
#define I2S_READ_LEN (16 * 1024)
#define RECORD_TIME (10)
#define I2S_CHANNEL_NUM (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

File file;
const char filename[] = "/recording.wav";
const int headerSize = 44;
bool isWIFIConnected = false;
bool isRecording = false;
bool shouldStopRecording = false;

bool lastButtonState = HIGH;
bool currentButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

const char *ssid = "DRKHOADANG";
const char *password = "1234Dang";
const char *serverURL = "http://192.168.1.11:8888/uploadAudio";

void setup()
{
    pinMode(BUTTON, INPUT_PULLUP);
    Serial.begin(115200);
    Serial.println("=== ESP32 Whisper Audio Recorder (Toggle Mode) ===");

    SPIFFSInit();
    i2sInit();
    connectWiFi();

    Serial.println("Nhấn nút để bắt đầu hoặc dừng ghi âm");
}

void loop()
{
    int reading = digitalRead(BUTTON);

    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (reading != currentButtonState)
        {
            currentButtonState = reading;

            if (currentButtonState == LOW)
            {
                if (!isRecording)
                {
                    Serial.println("*** Bắt đầu ghi âm ***");
                    startRecording();
                }
                else
                {
                    Serial.println("*** Dừng ghi âm ***");
                    shouldStopRecording = true;
                }
            }
        }
    }

    lastButtonState = reading;

    if (Serial.available())
    {
        String input = Serial.readString();
        input.trim();
        if (input.length() > 0 && !isRecording)
        {
            Serial.println("*** Test từ Serial - Ghi âm 5 giây ***");
            startRecording();
        }
    }

    delay(10);
}

void SPIFFSInit()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS khởi tạo thất bại!");
        while (1)
            yield();
    }
    Serial.println("SPIFFS khởi tạo thành công");
}

void i2sInit()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0,
        .dma_buf_count = 64,
        .dma_buf_len = 1024,
        .use_apll = 1};

    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = -1,
        .data_in_num = I2S_SD};

    i2s_set_pin(I2S_PORT, &pin_config);
    Serial.println("I2S khởi tạo thành công");
}

void connectWiFi()
{
    WiFi.begin(ssid, password);
    Serial.print("Đang kết nối WiFi");

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        isWIFIConnected = true;
        Serial.println();
        Serial.print("Đã kết nối WiFi! IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println();
        Serial.println("Không thể kết nối WiFi!");
        isWIFIConnected = false;
    }
}

void startRecording()
{
    if (!isWIFIConnected)
    {
        Serial.println("WiFi chưa kết nối! Thử lại...");
        connectWiFi();
        if (!isWIFIConnected)
        {
            Serial.println("Không thể ghi âm nếu không có WiFi!");
            return;
        }
    }

    isRecording = true;
    shouldStopRecording = false;
    xTaskCreate(i2s_adc, "i2s_adc", 1024 * 4, NULL, 1, NULL);
}

void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value = 0;
    for (int i = 0; i < len; i += 2)
    {
        dac_value = ((((uint16_t)(s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
        d_buff[j++] = 0;
        d_buff[j++] = dac_value * 256 / 2048;
    }
}

void i2s_adc(void *arg)
{
    SPIFFS.remove(filename);
    file = SPIFFS.open(filename, FILE_WRITE);
    if (!file)
    {
        Serial.println("Không thể tạo file!");
        isRecording = false;
        vTaskDelete(NULL);
        return;
    }

    byte header[headerSize];
    wavHeader(header, 0);
    file.write(header, headerSize);

    int i2s_read_len = I2S_READ_LEN;
    int flash_wr_size = 0;
    size_t bytes_read;

    char *i2s_read_buff = (char *)calloc(i2s_read_len, sizeof(char));
    uint8_t *flash_write_buff = (uint8_t *)calloc(i2s_read_len, sizeof(char));

    if (!i2s_read_buff || !flash_write_buff)
    {
        Serial.println("Không đủ RAM!");
        file.close();
        isRecording = false;
        vTaskDelete(NULL);
        return;
    }

    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

    Serial.println("*** Đang ghi âm... ***");
    unsigned long startTime = millis();

    while (!shouldStopRecording && flash_wr_size < FLASH_RECORD_SIZE)
    {
        i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
        i2s_adc_data_scale(flash_write_buff, (uint8_t *)i2s_read_buff, i2s_read_len);
        file.write((const byte *)flash_write_buff, i2s_read_len);
        flash_wr_size += i2s_read_len;

        unsigned long currentTime = millis();
        if ((currentTime - startTime) % 1000 < 100)
        {
            float recordedSeconds = (currentTime - startTime) / 1000.0;
            Serial.printf("Thời gian ghi: %.1f giây\n", recordedSeconds);
        }

        vTaskDelay(1);
    }

    file.seek(0);
    wavHeader(header, flash_wr_size);
    file.write(header, headerSize);
    file.close();

    unsigned long recordTime = millis() - startTime;
    float recordedSeconds = recordTime / 1000.0;

    Serial.printf("*** Ghi âm kết thúc sau %.1f giây ***\n", recordedSeconds);

    free(i2s_read_buff);
    free(flash_write_buff);

    if (recordedSeconds >= 0.5)
    {
        uploadFile();
    }
    else
    {
        Serial.println("Thời gian ghi quá ngắn, không upload");
        SPIFFS.remove(filename);
    }

    isRecording = false;
    shouldStopRecording = false;
    Serial.println("Nhấn nút để ghi âm tiếp...");
    vTaskDelete(NULL);
}

void wavHeader(byte *header, int wavSize)
{
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    unsigned int fileSize = wavSize + headerSize - 8;
    header[4] = (byte)(fileSize);
    header[5] = (byte)(fileSize >> 8);
    header[6] = (byte)(fileSize >> 16);
    header[7] = (byte)(fileSize >> 24);
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';
    header[16] = 0x10;
    header[17] = 0x00;
    header[18] = 0x00;
    header[19] = 0x00;
    header[20] = 0x01;
    header[21] = 0x00;
    header[22] = 0x01;
    header[23] = 0x00;
    header[24] = 0x80;
    header[25] = 0x3E;
    header[26] = 0x00;
    header[27] = 0x00;
    header[28] = 0x00;
    header[29] = 0x7D;
    header[30] = 0x01;
    header[31] = 0x00;
    header[32] = 0x02;
    header[33] = 0x00;
    header[34] = 0x10;
    header[35] = 0x00;
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';
    header[40] = (byte)(wavSize);
    header[41] = (byte)(wavSize >> 8);
    header[42] = (byte)(wavSize >> 16);
    header[43] = (byte)(wavSize >> 24);
}

void uploadFile()
{
    file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
        Serial.println("Không thể mở file để upload!");
        return;
    }

    Serial.println("=== Uploading lên Whisper server ===");
    Serial.printf("Kích thước file: %d bytes\n", file.size());

    HTTPClient client;
    client.begin(serverURL);
    client.addHeader("Content-Type", "audio/wav");
    client.setTimeout(30000);

    unsigned long uploadStart = millis();
    int httpResponseCode = client.sendRequest("POST", &file, file.size());
    unsigned long uploadTime = millis() - uploadStart;

    Serial.printf("HTTP Response: %d (Mất %lu ms)\n", httpResponseCode, uploadTime);

    if (httpResponseCode == 200)
    {
        String response = client.getString();
        Serial.println("==== PHẢN HỒI WHISPER ====");
        Serial.println(response);
    }
    else
    {
        Serial.println("Upload lỗi:");
        Serial.println(client.getString());
    }

    file.close();
    client.end();
    SPIFFS.remove(filename);
    Serial.println("Đã xóa file sau khi upload");
}
