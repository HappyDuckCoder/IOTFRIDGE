import os
import torch
import sounddevice as sd
import scipy.io.wavfile as wavfile
import numpy as np
from transformers import AutoProcessor, AutoModelForSpeechSeq2Seq, GenerationConfig

# === Class xử lý ghi âm ===
class Recorder:
    def __init__(self, duration=5, fs=16000, save_path="resources/audio/audio.wav"):
        self.duration = duration
        self.fs = fs
        self.save_path = save_path

    def record(self):
        print(f"Bắt đầu ghi âm {self.duration} giây...")
        recording = sd.rec(int(self.duration * self.fs), samplerate=self.fs, channels=1, dtype='float32')
        sd.wait()
        print("Ghi âm hoàn tất.")
        return recording

    def save(self, data):
        os.makedirs(os.path.dirname(self.save_path), exist_ok=True)
        audio_int16 = np.int16(data * 32767)
        wavfile.write(self.save_path, self.fs, audio_int16)
        print(f"Đã lưu vào: {self.save_path}")

    def record_to_file(self):
        audio = self.record()
        self.save(audio)

# === Class xử lý mô hình âm thanh ===
class AudioModel:
    def __init__(self, model_name="tranviethuy01/whisper-medium-vi", max_length=100):
        self.model_name = model_name
        self.max_length = max_length
        self.device = "cuda" if torch.cuda.is_available() else "cpu"
        self.processor = None
        self.model = None

    def load(self):
        print("Đang tải mô hình Whisper...")
        self.processor = AutoProcessor.from_pretrained(self.model_name)
        self.model = AutoModelForSpeechSeq2Seq.from_pretrained(self.model_name, torch_dtype=torch.float32)
        self.model.to(self.device)

        self.model.generation_config = GenerationConfig(
            max_length=self.max_length,
            pad_token_id=self.processor.tokenizer.pad_token_id,
            eos_token_id=self.processor.tokenizer.eos_token_id,
            decoder_start_token_id=self.model.config.decoder_start_token_id,
            use_cache=False
        )
        print("Mô hình Whisper đã tải.")

    def transcribe(self, wav_path):
        print(f"Đang đọc file: {wav_path}")
        sr, audio = wavfile.read(wav_path)
        if audio.dtype == np.int16:
            audio = audio.astype(np.float32) / 32767.0

        audio_input = audio.flatten()
        inputs = self.processor(audio_input, sampling_rate=sr, return_tensors="pt")
        input_features = inputs["input_features"].to(self.device)
        attention_mask = torch.ones(input_features.shape[:-1], dtype=torch.long).to(self.device)

        print("Đang nhận diện giọng nói...")
        with torch.no_grad():
            generated_ids = self.model.generate(
                input_features=input_features,
                attention_mask=attention_mask
            )

        return self.processor.batch_decode(generated_ids, skip_special_tokens=True)[0]

# === Hàm main ===
def main():
    recorder = Recorder()
    model = AudioModel()
    model.load()

    while True:
        cmd = input("\nNhập 'ok' để ghi âm (hoặc 'q' để thoát): ").strip().lower()
        if cmd == "ok":
            recorder.record_to_file()
            text = model.transcribe(recorder.save_path)
            print("\nKết quả nhận diện:")
            print(text)
        elif cmd == "q":
            print("Thoát chương trình.")
            break
        else:
            print("Vui lòng nhập 'ok' để ghi âm hoặc 'q' để thoát.")

if __name__ == "__main__":
    main()
