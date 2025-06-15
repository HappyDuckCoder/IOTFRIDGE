#include <driver/i2s.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (16000)
#define I2S_SAMPLE_BITS (16)
#define I2S_READ_LEN (16 * 1024)
#define RECORD_TIME (10) // Seconds
#define I2S_CHANNEL_NUM (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

#define I2S_WS 21
#define I2S_SD 23
#define I2S_SCK 22
#define BUTTON 19

const char *ssid = "DRKHOADANG";
const char *password = "1234Dang";
const char *serverURL = "http://192.168.1.11:8888/uploadAudio";

File file;
const char filename[] = "/recording.wav";
const int headerSize = 44;
bool isWIFIConnected = false;
bool isRecording = false;
bool buttonPressed = false;
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 200; // 200ms debounce

TaskHandle_t recordingTaskHandle = NULL;

void setup()
{
    pinMode(BUTTON, INPUT); // Nút nhấn tích cực HIGH

    Serial.begin(115200);
    SPIFFSInit();
    i2sInit();

    // Khởi tạo WiFi task
    xTaskCreate(wifiConnect, "wifi_Connect", 4096, NULL, 0, NULL);

    Serial.println("Hệ thống sẵn sàng. Nhấn nút để bắt đầu ghi âm.");
}

void loop()
{
    // Kiểm tra trạng thái nút nhấn
    if (digitalRead(BUTTON) == HIGH && !buttonPressed &&
        (millis() - lastButtonPress > debounceDelay))
    {

        buttonPressed = true;
        lastButtonPress = millis();

        if (!isRecording)
        {
            // Bắt đầu ghi âm
            startRecording();
        }
        else
        {
            // Dừng ghi âm
            stopRecording();
        }
    }

    // Reset trạng thái nút khi thả ra
    if (digitalRead(BUTTON) == LOW && buttonPressed)
    {
        buttonPressed = false;
    }

    delay(50); // Giảm tải CPU
}

void startRecording()
{
    if (isRecording)
        return;

    Serial.println("=== Bắt đầu ghi âm ===");
    isRecording = true;

    // Chuẩn bị file để ghi
    SPIFFS.remove(filename);
    file = SPIFFS.open(filename, FILE_WRITE);
    if (!file)
    {
        Serial.println("Không thể tạo file ghi âm!");
        isRecording = false;
        return;
    }

    // Ghi header WAV
    byte header[headerSize];
    wavHeader(header, FLASH_RECORD_SIZE);
    file.write(header, headerSize);

    // Tạo task ghi âm
    xTaskCreate(i2s_adc, "i2s_adc", 1024 * 4, NULL, 1, &recordingTaskHandle);
}

void stopRecording()
{
    if (!isRecording)
        return;

    Serial.println("=== Dừng ghi âm ===");
    isRecording = false;

    // Dừng task ghi âm
    if (recordingTaskHandle != NULL)
    {
        vTaskDelete(recordingTaskHandle);
        recordingTaskHandle = NULL;
    }

    // Đóng file
    if (file)
    {
        file.close();
    }

    listSPIFFS();

    // Upload file nếu WiFi đã kết nối
    if (isWIFIConnected)
    {
        uploadFile();
    }

    Serial.println("Nhấn nút để ghi âm tiếp theo.");
}

void SPIFFSInit()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS initialisation failed!");
        while (1)
            yield();
    }
    Serial.println("SPIFFS khởi tạo thành công.");
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
    Serial.println("I2S khởi tạo thành công.");
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
    int i2s_read_len = I2S_READ_LEN;
    int flash_wr_size = 0;
    size_t bytes_read;

    char *i2s_read_buff = (char *)calloc(i2s_read_len, sizeof(char));
    uint8_t *flash_write_buff = (uint8_t *)calloc(i2s_read_len, sizeof(char));

    if (!i2s_read_buff || !flash_write_buff)
    {
        Serial.println("Không đủ bộ nhớ để ghi âm!");
        if (i2s_read_buff)
            free(i2s_read_buff);
        if (flash_write_buff)
            free(flash_write_buff);
        isRecording = false;
        vTaskDelete(NULL);
        return;
    }

    // Clear I2S buffer
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

    Serial.println("*** Đang ghi âm ***");

    while (isRecording && flash_wr_size < FLASH_RECORD_SIZE)
    {
        // Đọc dữ liệu từ I2S
        esp_err_t result = i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, 100);

        if (result == ESP_OK && bytes_read > 0)
        {
            // Xử lý và ghi dữ liệu
            i2s_adc_data_scale(flash_write_buff, (uint8_t *)i2s_read_buff, i2s_read_len);
            file.write((const byte *)flash_write_buff, i2s_read_len);
            flash_wr_size += i2s_read_len;

            // Hiển thị tiến trình
            int progress = flash_wr_size * 100 / FLASH_RECORD_SIZE;
            if (progress % 10 == 0)
            { // Chỉ hiển thị mỗi 10%
                Serial.printf("Tiến trình ghi âm: %d%%\n", progress);
            }
        }

        // Yield để task khác có thể chạy
        vTaskDelay(1);
    }

    // Dọn dẹp bộ nhớ
    free(i2s_read_buff);
    free(flash_write_buff);

    if (isRecording)
    {
        Serial.println("*** Hoàn thành ghi âm (đạt thời gian tối đa) ***");
        isRecording = false;
        file.close();
        listSPIFFS();

        if (isWIFIConnected)
        {
            uploadFile();
        }
    }

    recordingTaskHandle = NULL;
    vTaskDelete(NULL);
}

void wavHeader(byte *header, int wavSize)
{
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';
    unsigned int fileSize = wavSize + headerSize - 8;
    header[4] = (byte)(fileSize & 0xFF);
    header[5] = (byte)((fileSize >> 8) & 0xFF);
    header[6] = (byte)((fileSize >> 16) & 0xFF);
    header[7] = (byte)((fileSize >> 24) & 0xFF);
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
    header[40] = (byte)(wavSize & 0xFF);
    header[41] = (byte)((wavSize >> 8) & 0xFF);
    header[42] = (byte)((wavSize >> 16) & 0xFF);
    header[43] = (byte)((wavSize >> 24) & 0xFF);
}

void listSPIFFS(void)
{
    Serial.println(F("\r\nDanh sách files trong SPIFFS:"));
    static const char line[] PROGMEM = "=================================================";

    Serial.println(FPSTR(line));
    Serial.println(F("  Tên file                              Kích thước"));
    Serial.println(FPSTR(line));

    fs::File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println(F("Không thể mở thư mục"));
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(F("Không phải thư mục"));
        return;
    }

    fs::File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("THƯ MỤC: ");
            String fileName = file.name();
            Serial.print(fileName);
        }
        else
        {
            String fileName = file.name();
            Serial.print("  " + fileName);
            int spaces = 33 - fileName.length();
            if (spaces < 1)
                spaces = 1;
            while (spaces--)
                Serial.print(" ");
            String fileSize = (String)file.size();
            spaces = 10 - fileSize.length();
            if (spaces < 1)
                spaces = 1;
            while (spaces--)
                Serial.print(" ");
            Serial.println(fileSize + " bytes");
        }
        file = root.openNextFile();
    }

    Serial.println(FPSTR(line));
    Serial.println();
}

void wifiConnect(void *pvParameters)
{
    Serial.println("Đang kết nối WiFi...");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500);
        Serial.print(".");
    }

    isWIFIConnected = true;
    Serial.println();
    Serial.print("WiFi đã kết nối! IP: ");
    Serial.println(WiFi.localIP());

    while (true)
    {
        vTaskDelay(1000);
    }
}

// Hàm mới để xóa tất cả file cũ trong SPIFFS
void clearOldFiles()
{
    Serial.println("=== Xóa các file cũ trong SPIFFS ===");

    fs::File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println("Không thể mở thư mục SPIFFS");
        return;
    }

    if (!root.isDirectory())
    {
        Serial.println("SPIFFS root không phải là thư mục");
        return;
    }

    // Lấy danh sách tất cả file cần xóa
    std::vector<String> filesToDelete;
    fs::File file = root.openNextFile();
    while (file)
    {
        if (!file.isDirectory())
        {
            String fileName = file.name();
            filesToDelete.push_back(fileName);
            Serial.println("Tìm thấy file: " + fileName);
        }
        file = root.openNextFile();
    }

    // Xóa từng file
    int deletedCount = 0;
    for (String fileName : filesToDelete)
    {
        Serial.println("Đang xóa: " + fileName);
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

    // Hiển thị dung lượng trống
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;

    Serial.printf("Dung lượng SPIFFS: %d bytes tổng, %d bytes đã dùng, %d bytes trống\n",
                  totalBytes, usedBytes, freeBytes);
    Serial.println("=== Hoàn thành xóa file cũ ===\n");
}

void uploadFile()
{
    file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
        Serial.println("FILE KHÔNG TỒN TẠI!");
        return;
    }

    Serial.println("===> Đang upload file lên server Node.js");

    HTTPClient client;
    client.begin(serverURL);
    client.addHeader("Content-Type", "audio/wav");
    int httpResponseCode = client.sendRequest("POST", &file, file.size());
    Serial.print("Mã phản hồi HTTP: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200)
    {
        String response = client.getString();
        Serial.println("==================== Phiên âm ====================");
        Serial.println(response);
        Serial.println("====================   Kết thúc   ====================");

        // Xóa các file cũ sau khi upload thành công
        file.close();
        client.end();
        clearOldFiles();
    }
    else
    {
        Serial.println("Lỗi upload");
        file.close();
        client.end();
    }
}