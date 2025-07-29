import wave

def convert_pcm_to_wav(pcm_path, wav_path, sample_rate=16000, num_channels=1, sample_width=2):
    with open(pcm_path, 'rb') as pcmfile:
        pcmdata = pcmfile.read()
    
    with wave.open(wav_path, 'wb') as wavfile:
        wavfile.setnchannels(num_channels)
        wavfile.setsampwidth(sample_width)
        wavfile.setframerate(sample_rate)
        wavfile.writeframes(pcmdata)

import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))

from thirdparty.api.getImageUrl import ImageSearchTool
from thirdparty.api.ClassificationFunction import Model, model_gemini_name

def get_image_url(food_name):
    query = food_name
    res = ImageSearchTool.google_image_search(query)
    return res[0]

def get_category(food_name):
    model = Model(model_gemini_name)
    model.runModel()
    prompt = f"từ {food_name}, hãy cho biết {food_name} thuộc category nào trong các loại sau: 'vitamin_and_fruits', 'protein', 'carbohydrate', 'fat'."
    response = model.generate_content(prompt)
    response_text = response.text.strip()
    return response_text

def get_calories(food_name):
    # 1 prompt: "vitamin_and_fruits", "protein", "carbohydrate", "fat"
    return 100

def main(): 
    print(get_category("sữa"))

if __name__ == "__main__":
    main()