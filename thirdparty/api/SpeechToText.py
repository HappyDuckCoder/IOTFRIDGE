# Thư viện xử lý hệ thống, âm thanh, mô hình AI
import os
import torch
import sounddevice as sd
import scipy.io.wavfile as wavfile
import numpy as np
from transformers import AutoProcessor, AutoModelForSpeechSeq2Seq, GenerationConfig

# Class xử lý ghi âm từ microphone ===
class Recorder:
    def __init__(self, duration=5, fs=16000, save_path="resources/audio/audio.wav"):
        self.duration = duration  # Thời gian ghi âm (giây)
        self.fs = fs              # Tần số lấy mẫu (Hz)
        self.save_path = save_path  # Đường dẫn lưu file âm thanh

    # Ghi âm và trả về dữ liệu âm thanh dạng mảng numpy
    def record(self):
        print(f"Bắt đầu ghi âm {self.duration} giây...")
        recording = sd.rec(int(self.duration * self.fs), samplerate=self.fs, channels=1, dtype='float32')
        sd.wait()
        print("Ghi âm hoàn tất.")
        return recording

    # Lưu âm thanh ra file WAV
    def save(self, data):
        os.makedirs(os.path.dirname(self.save_path), exist_ok=True)
        audio_int16 = np.int16(data * 32767)  # Chuyển sang định dạng 16-bit
        wavfile.write(self.save_path, self.fs, audio_int16)
        print(f"Đã lưu vào: {self.save_path}")

    # Ghi âm và lưu trực tiếp ra file
    def record_to_file(self):
        audio = self.record()
        self.save(audio)

# Class xử lý mô hình nhận diện giọng nói
class AudioModel:
    def __init__(self, model_name="tranviethuy01/whisper-medium-vi", max_length=100):
        self.model_name = model_name  # Tên mô hình Whisper
        self.max_length = max_length
        self.device = "cuda" if torch.cuda.is_available() else "cpu"  # Dùng GPU nếu có
        self.processor = None
        self.model = None

    # Tải mô hình và cấu hình sinh văn bản
    def load(self):
        print("Đang tải mô hình Whisper...")
        self.processor = AutoProcessor.from_pretrained(self.model_name)
        self.model = AutoModelForSpeechSeq2Seq.from_pretrained(self.model_name, torch_dtype=torch.float32)
        self.model.to(self.device)

        # Cấu hình sinh văn bản (text generation config)
        self.model.generation_config = GenerationConfig(
            max_length=self.max_length,
            pad_token_id=self.processor.tokenizer.pad_token_id,
            eos_token_id=self.processor.tokenizer.eos_token_id,
            decoder_start_token_id=self.model.config.decoder_start_token_id,
            use_cache=False
        )

    # Nhận diện giọng nói từ file WAV
    def transcribe(self, wav_path):
        print(f"Đang đọc file: {wav_path}")
        sr, audio = wavfile.read(wav_path)
        # Chuẩn hóa dữ liệu nếu là int16
        if audio.dtype == np.int16:
            audio = audio.astype(np.float32) / 32767.0

        # Xử lý âm thanh đầu vào cho mô hình
        audio_input = audio.flatten()
        inputs = self.processor(audio_input, sampling_rate=sr, return_tensors="pt")
        input_features = inputs["input_features"].to(self.device)
        attention_mask = torch.ones(input_features.shape[:-1], dtype=torch.long).to(self.device)

        print("Đang nhận diện giọng nói...")
        # Dự đoán kết quả (không cập nhật trọng số)
        with torch.no_grad():
            generated_ids = self.model.generate(
                input_features=input_features,
                attention_mask=attention_mask
            )

        # Chuyển kết quả thành văn bản
        return self.processor.batch_decode(generated_ids, skip_special_tokens=True)[0]

# Main function 
def main():
    recorder = Recorder()  # Tạo đối tượng ghi âm
    model = AudioModel()   # Tạo đối tượng mô hình nhận diện
    model.load()           # Tải mô hình

    # Vòng lặp nhận lệnh từ người dùng
    while True:
        cmd = input("\nNhập 'ok' để ghi âm (hoặc 'q' để thoát): ").strip().lower()
        if cmd == "ok":
            recorder.record_to_file()                         # Ghi âm và lưu file
            text = model.transcribe(recorder.save_path)       # Nhận diện giọng nói
            print("\nKết quả nhận diện:")
            print(text)
        elif cmd == "q":
            print("Thoát chương trình.")
            break
        else:
            print("Vui lòng nhập 'ok' để ghi âm hoặc 'q' để thoát.")

if __name__ == "__main__":
    main()
