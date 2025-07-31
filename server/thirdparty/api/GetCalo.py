import requests
from my_util import translate_vietnamese_to_english

api_key = "Q3Fip13CRKZgjCsMFqgdySadqTBTyQcg0XFJWida"
base_url = "https://api.nal.usda.gov/fdc/v1/foods/search"

class GetCaloService:
    def __init__(self):
        pass
    def get_calo_usda(self, food_name: str):
        if not food_name or not food_name.strip():
            return None
            
        # Chuẩn hóa tên thực phẩm
        search_name = food_name.lower().strip()
        
        # Thử dịch sang tiếng Anh nếu là tiếng Việt
        # english_name = vietnamese_to_english.get(search_name, search_name)
        english_name = translate_vietnamese_to_english(search_name)
        
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
                        return round(float(value), 1)
            
            return 0
            
        except requests.RequestException as e:
            print(f"Lỗi khi gọi USDA API cho '{food_name}': {e}")
            return 0
        except Exception as e:
            print(f"Lỗi xử lý dữ liệu USDA cho '{food_name}': {e}")
            return 0

    def get_calo_with_quantity(self, food_name: str, quantity: float, unit: str):
        # Lấy calo per 100g
        calo_per_100g = self.get_calo_usda(food_name)
        
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
        
        return total_calories

if __name__ == "__main__":
    service = GetCaloService()

    test_foods = ["apple", "táo", "thịt bò", "chicken", "chuối", "rice"]
    
    for food in test_foods:
        print(f"\nTesting: {food}")
        calories = service.get_calo_usda(food)
        if calories:
            print(f"Calo: {calories} kcal/100g")
            
            # Test với số lượng cụ thể
            if food in ["táo", "apple"]:
                detail = service.get_calo_with_quantity(food, 2, "quả")
                if detail:
                    print(f"   2 quả = {detail['total_calories']} kcal")
        else:
            print(f"Không tìm thấy")