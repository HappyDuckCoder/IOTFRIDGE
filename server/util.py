import wave

# 🟢 Tạo WAV từ PCM
def convert_pcm_to_wav(pcm_path, wav_path, sample_rate=16000, num_channels=1, sample_width=2):
    with open(pcm_path, 'rb') as pcmfile:
        pcmdata = pcmfile.read()
    
    with wave.open(wav_path, 'wb') as wavfile:
        wavfile.setnchannels(num_channels)
        wavfile.setsampwidth(sample_width)
        wavfile.setframerate(sample_rate)
        wavfile.writeframes(pcmdata)