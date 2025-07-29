import wave
import random
import string
from datetime import datetime, timedelta
from thirdparty.api.getImageUrl import ImageSearchTool
from thirdparty.api.NLPapi import Model, model_gemini_name
from thirdparty.api.GetCalo import GetCaloService

def convert_pcm_to_wav(pcm_path, wav_path, sample_rate=16000, num_channels=1, sample_width=2):
    with open(pcm_path, 'rb') as pcmfile:
        pcmdata = pcmfile.read()
    
    with wave.open(wav_path, 'wb') as wavfile:
        wavfile.setnchannels(num_channels)
        wavfile.setsampwidth(sample_width)
        wavfile.setframerate(sample_rate)
        wavfile.writeframes(pcmdata)

def get_image_url(food_name):
    query = food_name
    res = ImageSearchTool.google_image_search(query)
    return res[0]

def get_category(food_name):
    model = Model(model_gemini_name)
    model.runModel()
    prompt = f"""
từ {food_name}, hãy cho biết {food_name} thuộc category nào trong các loại sau: 'vitamin_and_fruits', 'protein', 'carbohydrate', 'fat'.
Chỉ trả về 1 giá trị duy nhất trong các loại: 'vitamin_and_fruits', 'protein', 'carbohydrate', 'fat'.
"""
    response = model.generate_content(prompt)
    response_text = response.text.strip()
    return response_text

def get_calo_usda(food_name, quantity, unit): 
    service = GetCaloService()
    return service.get_calo_with_quantity(food_name, quantity, unit)

def generate_random_id_string(length=20):
    characters = string.ascii_letters + string.digits  # A-Z, a-z, 0-9
    return ''.join(random.choice(characters) for _ in range(length))

def get_default_expired_date(days=7):
    return (datetime.now() + timedelta(days=days)).date()
