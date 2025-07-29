import wave
import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))
import requests
import json
from typing import Optional, Dict, Any, List
import time

from thirdparty.api.getImageUrl import ImageSearchTool
from thirdparty.api.ClassificationFunction import Model, model_gemini_name

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
    prompt = f"từ {food_name}, hãy cho biết {food_name} thuộc category nào trong các loại sau: 'vitamin_and_fruits', 'protein', 'carbohydrate', 'fat'."
    response = model.generate_content(prompt)
    response_text = response.text.strip()
    return response_text

def get_calo_usda(food_name: str, api_key: str = "Q3Fip13CRKZgjCsMFqgdySadqTBTyQcg0XFJWida") -> Optional[float]:
    if not food_name or not food_name.strip():
        return None
    
    # Dictionary để dịch từ tiếng Việt sang tiếng Anh
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
    
    # Chuẩn hóa tên thực phẩm
    search_name = food_name.lower().strip()
    
    # Thử dịch sang tiếng Anh nếu là tiếng Việt
    english_name = vietnamese_to_english.get(search_name, search_name)
    
    base_url = "https://api.nal.usda.gov/fdc/v1/foods/search"
    
    params = {
        "query": english_name,
        "api_key": api_key,
        "pageSize": 10,
        "dataType": ["Foundation", "SR Legacy", "Survey (FNDDS)"],
        "sortBy": "dataType.keyword",
        "sortOrder": "asc"
    }
    
    try:
        response = requests.get(base_url, params=params, timeout=15)
        response.raise_for_status()
        
        data = response.json()
        foods = data.get("foods", [])
        
        if not foods:
            # Nếu không tìm thấy với tên tiếng Anh, thử với tên gốc
            if english_name != search_name:
                params["query"] = search_name
                response = requests.get(base_url, params=params, timeout=15)
                response.raise_for_status()
                data = response.json()
                foods = data.get("foods", [])
        
        # Tìm kiếm thông tin calo trong các kết quả
        for food_item in foods:
            nutrients = food_item.get("foodNutrients", [])
            
            for nutrient in nutrients:
                nutrient_id = nutrient.get("nutrientId")
                value = nutrient.get("value")
                
                # Nutrient ID 1008 = Energy (kcal)
                if nutrient_id == 1008 and value:
                    # Làm tròn đến 1 chữ số thập phân
                    return round(float(value), 1)
        
        # Nếu không tìm thấy trong USDA, trả về giá trị ước tính
        return get_estimated_calories(food_name)
        
    except requests.RequestException as e:
        print(f"Lỗi khi gọi USDA API cho '{food_name}': {e}")
        return get_estimated_calories(food_name)
    except Exception as e:
        print(f"Lỗi xử lý dữ liệu USDA cho '{food_name}': {e}")
        return get_estimated_calories(food_name)

def get_estimated_calories(food_name: str) -> Optional[float]:
    """
    Trả về giá trị calo ước tính cho các thực phẩm phổ biến
    khi không tìm thấy trong USDA API
    
    Args:
        food_name (str): Tên thực phẩm
        
    Returns:
        float: Số calo ước tính per 100g
    """
    # Database calo ước tính cho thực phẩm Việt Nam (kcal/100g)
    estimated_calories = {
        # Trái cây
        "táo": 52, "apple": 52,
        "chuối": 89, "banana": 89,
        "cam": 47, "orange": 47,
        "nho": 62, "grape": 62,
        "dưa hấu": 30, "watermelon": 30,
        "dứa": 50, "pineapple": 50,
        "xoài": 60, "mango": 60,
        "đu đủ": 43, "papaya": 43,
        
        # Thịt
        "thịt bò": 250, "beef": 250,
        "thịt heo": 242, "pork": 242,
        "thịt gà": 239, "chicken": 239,
        
        # Hải sản
        "cá hồi": 208, "salmon": 208,
        "cá": 150, "fish": 150,
        "tôm": 106, "shrimp": 106,
        "cua": 87, "crab": 87,
        
        # Rau củ
        "cà chua": 18, "tomato": 18,
        "cà rốt": 41, "carrot": 41,
        "khoai tây": 77, "potato": 77,
        "khoai lang": 86, "sweet potato": 86,
        "rau cải": 25, "cabbage": 25,
        "bắp": 96, "corn": 96,
        
        # Ngũ cốc
        "gạo": 365, "rice": 365,
        "bánh mì": 265, "bread": 265,
        
        # Sữa và sản phẩm từ sữa
        "sũa": 42, "milk": 42,
        "phô mai": 113, "cheese": 113,
        
        # Khác
        "trứng": 155, "egg": 155,
        "đậu phụ": 76, "tofu": 76,
    }
    
    search_name = food_name.lower().strip()
    
    # Tìm kiếm chính xác
    if search_name in estimated_calories:
        return float(estimated_calories[search_name])
    
    # Tìm kiếm gần đúng
    for food, calories in estimated_calories.items():
        if food in search_name or search_name in food:
            return float(calories)
    
    # Nếu không tìm thấy, trả về None
    print(f"Không tìm thấy thông tin calo cho '{food_name}'")
    return None

def get_calo_with_quantity(food_name: str, quantity: float, unit: str) -> Optional[Dict[str, Any]]:
    """
    Tính tổng calo dựa trên số lượng và đơn vị cụ thể
    
    Args:
        food_name (str): Tên thực phẩm
        quantity (float): Số lượng
        unit (str): Đơn vị (gam, kg, quả, etc.)
        
    Returns:
        Dict chứa thông tin calo tổng
    """
    # Lấy calo per 100g
    calo_per_100g = get_calo_usda(food_name)
    
    if not calo_per_100g:
        return None
    
    # Chuyển đổi về gram
    quantity_in_grams = quantity
    
    if unit.lower() in ["kg", "kilogram"]:
        quantity_in_grams = quantity * 1000
    elif unit.lower() in ["quả", "trái", "cái"]:
        # Trọng lượng trung bình của một số thực phẩm (gram)
        average_weights = {
            "táo": 150, "apple": 150,
            "chuối": 120, "banana": 120,
            "cam": 200, "orange": 200,
            "trứng": 50, "egg": 50,
            "cà chua": 100, "tomato": 100,
        }
        
        search_name = food_name.lower().strip()
        weight_per_item = 100  # Default weight
        
        for food, weight in average_weights.items():
            if food in search_name or search_name in food:
                weight_per_item = weight
                break
        
        quantity_in_grams = quantity * weight_per_item
    
    # Tính tổng calo
    total_calories = (calo_per_100g * quantity_in_grams) / 100
    
    return {
        "food_name": food_name,
        "quantity": quantity,
        "unit": unit,
        "quantity_in_grams": quantity_in_grams,
        "calo_per_100g": calo_per_100g,
        "total_calories": round(total_calories, 1)
    }

def test_calo_api():
    """
    Hàm test để kiểm tra API calo
    """
    test_foods = ["apple", "táo", "thịt bò", "chicken", "chuối", "rice"]
    
    print("=== Test USDA Calorie API ===")
    for food in test_foods:
        print(f"\nTesting: {food}")
        calories = get_calo_usda(food)
        if calories:
            print(f"✅ Calo: {calories} kcal/100g")
            
            # Test với số lượng cụ thể
            if food in ["táo", "apple"]:
                detail = get_calo_with_quantity(food, 2, "quả")
                if detail:
                    print(f"   2 quả = {detail['total_calories']} kcal")
        else:
            print(f"❌ Không tìm thấy")

def main(): 
    print(get_category("sữa"))
    print("\n" + "="*40)
    test_calo_api()

if __name__ == "__main__":
    main()