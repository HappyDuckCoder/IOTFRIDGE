/*
 * ESP32 AUDIO RECORDING AND UPLOAD SYSTEM
 * Hệ thống ghi âm và upload file âm thanh lên server
 *
 * Chức năng chính:
 * - Ghi âm thông qua I2S microphone
 * - Lưu file WAV vào SPIFFS
 * - Upload file lên server Node.js qua HTTP
 * - Điều khiển bằng nút nhấn
 */

#include <driver/i2s.h> // Thư viện I2S để ghi âm
#include <SPIFFS.h>     // Thư viện file system
#include <WiFi.h>       // Thư viện WiFi
#include <HTTPClient.h> // Thư viện HTTP client để upload

// ============== CẤU HÌNH I2S (GIAO TIẾP MICROPHONE) ==============
#define I2S_PORT I2S_NUM_0       // Sử dụng cổng I2S số 0
#define I2S_SAMPLE_RATE (16000)  // Tần số lấy mẫu: 16kHz (chuẩn cho voice)
#define I2S_SAMPLE_BITS (16)     // Độ phân giải: 16-bit
#define I2S_READ_LEN (16 * 1024) // Kích thước buffer đọc: 16KB
#define RECORD_TIME (10)         // Thời gian ghi tối đa: 10 giây
#define I2S_CHANNEL_NUM (1)      // Mono (1 kênh)

// Tính toán kích thước file ghi âm tối đa
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

// ============== CẤU HÌNH CHÂN KẾT NỐI ==============
#define I2S_WS 21  // Word Select (Left/Right Clock)
#define I2S_SD 23  // Serial Data (dữ liệu âm thanh)
#define I2S_SCK 22 // Serial Clock (xung clock)
#define BUTTON 19  // Chân nút nhấn

// ============== CẤU HÌNH WIFI VÀ SERVER ==============
const char *ssid = "DRKHOADANG";                                // Tên WiFi
const char *password = "1234Dang";                              // Mật khẩu WiFi
const char *serverURL = "http://192.168.1.11:8888/uploadAudio"; // URL server Node.js

// ============== BIẾN TOÀN CỤC ==============
File file;                                // Object file để ghi dữ liệu
const char filename[] = "/recording.wav"; // Tên file ghi âm
const int headerSize = 44;                // Kích thước header WAV (44 bytes chuẩn)

// Biến trạng thái hệ thống
bool isWIFIConnected = false;            // Trạng thái kết nối WiFi
bool isRecording = false;                // Trạng thái đang ghi âm
bool buttonPressed = false;              // Trạng thái nút nhấn
unsigned long lastButtonPress = 0;       // Thời điểm nhấn nút cuối
const unsigned long debounceDelay = 200; // Thời gian chống dội nút (200ms)

TaskHandle_t recordingTaskHandle = NULL; // Handle để quản lý task ghi âm

// ============== HÀM SETUP - KHỞI TẠO HỆ THỐNG ==============
void setup()
{
    // Cấu hình chân nút nhấn là INPUT (logic HIGH khi nhấn)
    pinMode(BUTTON, INPUT);

    // Khởi tạo Serial monitor
    Serial.begin(115200);

    // Khởi tạo các thành phần hệ thống
    SPIFFSInit(); // Khởi tạo file system
    i2sInit();    // Khởi tạo I2S cho microphone

    // Tạo task riêng để kết nối WiFi (không block main loop)
    xTaskCreate(wifiConnect, "wifi_Connect", 4096, NULL, 0, NULL);

    Serial.println("Hệ thống sẵn sàng. Nhấn nút để bắt đầu ghi âm.");
}

// ============== VÒNG LẶP CHÍNH - XỬ LÝ NÚT NHẤN ==============
void loop()
{
    // Kiểm tra trạng thái nút nhấn với chống dội
    if (digitalRead(BUTTON) == HIGH && !buttonPressed &&
        (millis() - lastButtonPress > debounceDelay))
    {

        buttonPressed = true;
        lastButtonPress = millis();

        if (!isRecording)
        {
            // Nếu chưa ghi âm -> Bắt đầu ghi âm
            startRecording();
        }
        else
        {
            // Nếu đang ghi âm -> Dừng ghi âm
            stopRecording();
        }
    }

    // Reset trạng thái nút khi thả ra
    if (digitalRead(BUTTON) == LOW && buttonPressed)
    {
        buttonPressed = false;
    }

    delay(50); // Delay nhỏ để giảm tải CPU
}

// ============== HÀM BẮT ĐẦU GHI ÂM ==============
void startRecording()
{
    if (isRecording)
        return; // Tránh ghi âm trùng lặp

    Serial.println("=== Bắt đầu ghi âm ===");
    isRecording = true;

    // Xóa file cũ và tạo file mới
    SPIFFS.remove(filename);
    file = SPIFFS.open(filename, FILE_WRITE);
    if (!file)
    {
        Serial.println("Không thể tạo file ghi âm!");
        isRecording = false;
        return;
    }

    // Tạo task riêng để xử lý ghi âm (không block main loop)
    xTaskCreate(i2s_adc, "i2s_adc", 1024 * 4, NULL, 1, &recordingTaskHandle);
}

// ============== HÀM DỪNG GHI ÂM ==============
void stopRecording()
{
    if (!isRecording)
        return; // Tránh dừng khi chưa ghi âm

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

    // Hiển thị danh sách file trong SPIFFS
    listSPIFFS();

    // Upload file nếu WiFi đã kết nối
    if (isWIFIConnected)
    {
        uploadFile();
    }

    Serial.println("Nhấn nút để ghi âm tiếp theo.");
}

// ============== KHỞI TẠO SPIFFS FILE SYSTEM ==============
void SPIFFSInit()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS initialisation failed!");
        while (1)
            yield(); // Dừng chương trình nếu SPIFFS lỗi
    }
    Serial.println("SPIFFS khởi tạo thành công.");
}

// ============== KHỞI TẠO I2S CHO MICROPHONE ==============
void i2sInit()
{
    // Cấu hình I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),       // Master mode, receive data
        .sample_rate = I2S_SAMPLE_RATE,                            // Tần số lấy mẫu
        .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS), // Độ phân giải
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,               // Chỉ sử dụng kênh trái
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0, // Không có flag đặc biệt
        .dma_buf_count = 64,   // Số buffer DMA
        .dma_buf_len = 1024,   // Kích thước mỗi buffer
        .use_apll = 1          // Sử dụng APLL để có độ chính xác cao
    };

    // Cài đặt driver I2S
    i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

    // Cấu hình chân kết nối I2S
    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK, // Serial Clock
        .ws_io_num = I2S_WS,   // Word Select
        .data_out_num = -1,    // Không sử dụng (chỉ thu âm)
        .data_in_num = I2S_SD  // Serial Data Input
    };

    i2s_set_pin(I2S_PORT, &pin_config);
    Serial.println("I2S khởi tạo thành công.");
}

// ============== HÀM CHUYỂN ĐỔI DỮ LIỆU I2S ==============
/*
 * Chuyển đổi dữ liệu thô từ I2S thành format phù hợp cho WAV
 * s_buff: buffer nguồn (dữ liệu thô từ I2S)
 * d_buff: buffer đích (dữ liệu đã xử lý)
 * len: độ dài dữ liệu
 */
void i2s_adc_data_scale(uint8_t *d_buff, uint8_t *s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value = 0;

    // Xử lý từng cặp byte (16-bit sample)
    for (int i = 0; i < len; i += 2)
    {
        // Ghép 2 byte thành 1 sample 16-bit
        dac_value = ((((uint16_t)(s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));

        // Chuyển đổi và scale dữ liệu
        d_buff[j++] = 0;                      // Byte thấp = 0
        d_buff[j++] = dac_value * 256 / 2048; // Byte cao = scale value
    }
}

// ============== TASK GHI ÂM CHÍNH ==============
/*
 * Task này chạy trong thread riêng để xử lý việc ghi âm
 * Đọc dữ liệu từ I2S và ghi vào file WAV
 */
void i2s_adc(void *arg)
{
    int i2s_read_len = I2S_READ_LEN; // Kích thước buffer đọc
    int flash_wr_size = 0;           // Tổng dữ liệu đã ghi
    size_t bytes_read;               // Số byte thực tế đọc được

    // Cấp phát bộ nhớ cho buffer
    char *i2s_read_buff = (char *)calloc(i2s_read_len, sizeof(char));          // Buffer đọc I2S
    uint8_t *flash_write_buff = (uint8_t *)calloc(i2s_read_len, sizeof(char)); // Buffer ghi file

    // Kiểm tra cấp phát bộ nhớ
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

    // Xóa buffer I2S cũ (đọc và bỏ dữ liệu cũ)
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

    Serial.println("*** Đang ghi âm ***");

    // Vòng lặp ghi âm chính
    while (isRecording && flash_wr_size < FLASH_RECORD_SIZE)
    {
        // Đọc dữ liệu từ I2S
        esp_err_t result = i2s_read(I2S_PORT, (void *)i2s_read_buff, i2s_read_len, &bytes_read, 100);

        if (result == ESP_OK && bytes_read > 0)
        {
            // Xử lý dữ liệu và ghi vào file
            i2s_adc_data_scale(flash_write_buff, (uint8_t *)i2s_read_buff, i2s_read_len);
            file.write((const byte *)flash_write_buff, i2s_read_len);
            flash_wr_size += i2s_read_len;

            // Hiển thị tiến trình (mỗi 10%)
            int progress = flash_wr_size * 100 / FLASH_RECORD_SIZE;
            if (progress % 10 == 0)
            {
                Serial.printf("Tiến trình ghi âm: %d%%\n", progress);
            }
        }

        // Yield để các task khác có thể chạy
        vTaskDelay(1);
    }

    // Dọn dẹp bộ nhớ
    free(i2s_read_buff);
    free(flash_write_buff);

    // Xử lý khi ghi âm hoàn thành (do hết thời gian)
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

    // Xóa task handle và kết thúc task
    recordingTaskHandle = NULL;
    vTaskDelete(NULL);
}

// ============== HIỂN THỊ DANH SÁCH FILE TRONG SPIFFS ==============
void listSPIFFS(void)
{
    Serial.println(F("\r\nDanh sách files trong SPIFFS:"));
    static const char line[] PROGMEM = "=================================================";

    Serial.println(FPSTR(line));
    Serial.println(F("  Tên file                              Kích thước"));
    Serial.println(FPSTR(line));

    // Mở thư mục root của SPIFFS
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

    // Duyệt qua tất cả file
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
            // Hiển thị thông tin file với format đẹp
            String fileName = file.name();
            Serial.print("  " + fileName);

            // Căn chỉnh cột
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

// ============== TASK KẾT NỐI WIFI ==============
/*
 * Task chạy riêng để kết nối WiFi
 * Chạy trong thread riêng để không block main loop
 */
void wifiConnect(void *pvParameters)
{
    Serial.println("Đang kết nối WiFi...");
    WiFi.begin(ssid, password);

    // Chờ kết nối WiFi
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500); // Delay 500ms
        Serial.print(".");
    }

    // Kết nối thành công
    isWIFIConnected = true;
    Serial.println();
    Serial.print("WiFi đã kết nối! IP: ");
    Serial.println(WiFi.localIP());

    // Task tiếp tục chạy để duy trì kết nối
    while (true)
    {
        vTaskDelay(1000);
    }
}

// ============== XÓA TẤT CẢ FILE CŨ TRONG SPIFFS ==============
/*
 * Xóa tất cả file trong SPIFFS để giải phóng bộ nhớ
 * Được gọi sau khi upload thành công
 */
void clearOldFiles()
{
    Serial.println("=== Xóa các file cũ trong SPIFFS ===");

    // Mở thư mục root
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

    // Hiển thị thông tin dung lượng
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;

    Serial.printf("Dung lượng SPIFFS: %d bytes tổng, %d bytes đã dùng, %d bytes trống\n",
                  totalBytes, usedBytes, freeBytes);
    Serial.println("=== Hoàn thành xóa file cũ ===\n");
}

// ============== UPLOAD FILE LÊN SERVER ==============
/*
 * Upload file WAV lên server Node.js qua HTTP POST
 * Server sẽ xử lý và trả về kết quả phiên âm
 */
void uploadFile()
{
    // Mở file để đọc
    file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
        Serial.println("FILE KHÔNG TỒN TẠI!");
        return;
    }

    Serial.println("===> Đang upload file lên server python");

    // Tạo HTTP client
    HTTPClient client;
    client.begin(serverURL);
    client.addHeader("Content-Type", "audio/wav"); // Set content type

    // Gửi POST request với file data
    int httpResponseCode = client.sendRequest("POST", &file, file.size());
    Serial.print("Mã phản hồi HTTP: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200)
    {
        // Upload thành công - hiển thị kết quả phiên âm
        String response = client.getString();
        Serial.println("==================== Phiên âm ====================");
        Serial.println(response);
        Serial.println("====================   Kết thúc   ====================");

        // Dọn dẹp và xóa file cũ
        file.close();
        client.end();
        clearOldFiles(); // Xóa file để tiết kiệm bộ nhớ
    }
    else
    {
        // Upload thất bại
        Serial.println("Lỗi upload");
        file.close();
        client.end();
    }
}