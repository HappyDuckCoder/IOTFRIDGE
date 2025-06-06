#include <driver/i2s.h>

#define I2S_WS 15
#define I2S_SD 32
#define I2S_SCK 14

void setupI2SMic()
{
    const i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000, // 16kHz
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024};

    const i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD};

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_0);
}

void setup()
{
    Serial.begin(115200);
    setupI2SMic();
}

void loop()
{
    // Buffer để lưu âm thanh
    const int bufferSize = 1024;
    int16_t buffer[bufferSize];
    size_t bytesRead;

    i2s_read(I2S_NUM_0, &buffer, bufferSize * sizeof(int16_t), &bytesRead, portMAX_DELAY);

    // Gửi dữ liệu dạng RAW lên máy tính qua Serial (sẽ xử lý tiếp bên PC)
    Serial.write((uint8_t *)buffer, bytesRead);
}
