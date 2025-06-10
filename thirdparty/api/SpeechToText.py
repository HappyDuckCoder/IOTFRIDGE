import torch
import sounddevice as sd
from transformers import AutoProcessor, AutoModelForSpeechSeq2Seq

DURATION = 10
FS = 16000
model_name = "tranviethuy01/whisper-medium-vi"

def Record(): 
    print("Bắt đầu ghi âm 10 giây...")
    recording = sd.rec(int(DURATION * FS), samplerate=FS, channels=1, dtype='float32')
    sd.wait()
    print("Ghi âm hoàn tất.")

    return recording

def main():
    recording = Record()

    audio_input = recording.flatten()

    print(audio_input)

    print("Đang tải mô hình...")
    processor = AutoProcessor.from_pretrained(model_name)
    model = AutoModelForSpeechSeq2Seq.from_pretrained(
        model_name,
        torch_dtype=torch.float32  # Không dùng float16
    )

    device = "cuda" if torch.cuda.is_available() else "cpu"
    model.to(device)

    inputs = processor(audio_input, sampling_rate=FS, return_tensors="pt").to(device)

    print("Đang nhận diện giọng nói...")
    with torch.no_grad():
        generated_ids = model.generate(inputs["input_features"])

    transcription = processor.batch_decode(generated_ids, skip_special_tokens=True)[0]

    print("Kết quả nhận diện:")
    print(transcription)
    
if __name__ == "__main__":
    main()