import wave
import random
import string
from datetime import datetime, timedelta
from thirdparty.api.getImageUrl import ImageSearchTool
from thirdparty.api.NLPapi import Model, model_gemini_name

vietnamese_to_english = {
    "táo": "apple",
    "chuối": "banana", 
    "cam": "orange",
    "thịt bò": "beef",
    "thịt heo": "pork",
    "thịt gà": "chicken",
    "cá hồi": "salmon",
    "cá": "fish",
    "tôm": "shrimp",
    "cua": "crab",
    "cà chua": "tomato",
    "cà rốt": "carrot",
    "khoai tây": "potato",
    "khoai lang": "sweet potato",
    "gạo": "rice",
    "bánh mì": "bread",
    "sữa": "milk",
    "trứng": "egg",
    "đậu phụ": "tofu",
    "rau cải": "cabbage",
    "rau muống": "water spinach",
    "bắp": "corn",
    "đậu xanh": "mung bean",
    "ớt": "pepper",
    "hành": "onion",
    "tỏi": "garlic",
    "gừng": "ginger",
    "nho": "grape",
    "dưa hấu": "watermelon",
    "dứa": "pineapple",
    "xoài": "mango",
    "đu đủ": "papaya",
    "bơ": "avocado",
    "pho mai": "cheese",
    "phô mai": "cheese",
    "bơ thực vật": "butter",
    "dầu ăn": "oil",
    "đường": "sugar",
    "muối": "salt"
}

def translate_vietnamese_to_english(name):
    return vietnamese_to_english.get(name, name)

def convert_to_datetime(date_obj):
    if date_obj is None:
        return None
    if isinstance(date_obj, datetime):
        return date_obj.replace(tzinfo=None)
    if hasattr(date_obj, "to_datetime"):
        return date_obj.to_datetime().replace(tzinfo=None)
    if isinstance(date_obj, str):
        try:
            return datetime.fromisoformat(date_obj).replace(tzinfo=None)
        except:
            return None
    return date_obj

def convert_pcm_to_wav(pcm_path, wav_path, sample_rate=16000, num_channels=1, sample_width=2):
    with open(pcm_path, 'rb') as pcmfile:
        pcmdata = pcmfile.read()
    
    # Kiểm tra dữ liệu có đúng không
    print(f"PCM file size: {len(pcmdata)} bytes")
    print(f"Expected samples: {len(pcmdata) // sample_width}")
    
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

def generate_random_id_string(length=20):
    characters = string.ascii_letters + string.digits  # A-Z, a-z, 0-9
    return ''.join(random.choice(characters) for _ in range(length))

def get_default_expired_date(days=7):
    return (datetime.now() + timedelta(days=days)).date()
