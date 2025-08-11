import requests
import re
from typing import List, Optional
from datetime import datetime
from thirdparty.database.model import Recipe, Food

API_KEY = "73459d9516744851b2a4f7296cf675f5"
API_URL = "https://api.spoonacular.com/recipes/complexSearch"

class GetRecipeService:
    def __init__(self):
        pass

    @staticmethod
    def get_list_recipe_by_spoonacular(ingredients: List[str], diet: str, number: int = 10) -> List[Recipe]:
        try:
            # Tạo chuỗi ingredients từ danh sách Food
            ingredients_str = ",".join(ingredients)
            print("tìm recipes với: ", ingredients_str)
            
            # Tham số cho API
            params = {
                'apiKey': API_KEY,
                'includeIngredients': ingredients_str,
                'diet': diet,
                'number': number,
                'addRecipeInformation': True,
                'fillIngredients': True,
                'addRecipeNutrition': True,
                'instructionsRequired': True,
                'sort': 'max-used-ingredients'  # Ưu tiên công thức dùng nhiều nguyên liệu có sẵn nhất
            }
            
            # Gọi API
            response = requests.get(API_URL, params=params)
            response.raise_for_status()
            
            data = response.json()
            recipes = []
            
            # Xử lý kết quả từ API
            if 'results' in data:
                for recipe_data in data['results']:
                    recipe = GetRecipeService._convert_spoonacular_to_recipe(recipe_data)
                    if recipe:
                        recipes.append(recipe)
            
            return recipes
            
        except requests.exceptions.RequestException as e:
            print(f"Lỗi khi gọi Spoonacular API: {e}")
            return []
        except Exception as e:
            print(f"Lỗi không xác định: {e}")
            return []
    
    @staticmethod
    def _convert_spoonacular_to_recipe(spoonacular_data: dict) -> Optional[Recipe]:
        try:
            # Lấy thông tin cơ bản
            recipe_id = str(spoonacular_data.get('id', ''))
            title = spoonacular_data.get('title', 'Không có tiêu đề')
            cook_time = spoonacular_data.get('readyInMinutes', 30)
            people_serving = spoonacular_data.get('servings', 1)
            image_url = spoonacular_data.get('image', '')
            
            # Xác định độ khó dựa trên thời gian nấu
            if cook_time <= 20:
                difficulty = "easy"
            elif cook_time <= 45:
                difficulty = "medium"
            else:
                difficulty = "hard"
            
            # Lấy hướng dẫn nấu ăn
            instructions = GetRecipeService._extract_instructions(spoonacular_data)
            
            return Recipe(
                id=recipe_id,
                title=title,
                cook_time=cook_time,
                people_serving=people_serving,
                difficulty=difficulty,
                instructions=instructions,
                image_url=image_url
            )
            
        except Exception as e:
            print(f"Lỗi khi chuyển đổi dữ liệu công thức {spoonacular_data.get('id', 'unknown')}: {e}")
            return None
    
    @staticmethod
    def _extract_instructions(spoonacular_data: dict) -> str:
        instructions = ""
        
        # Thử lấy từ analyzedInstructions trước
        if 'analyzedInstructions' in spoonacular_data and spoonacular_data['analyzedInstructions']:
            steps = []
            for instruction_group in spoonacular_data['analyzedInstructions']:
                if 'steps' in instruction_group:
                    for step in instruction_group['steps']:
                        step_number = step.get('number', len(steps) + 1)
                        step_text = step.get('step', '')
                        if step_text:
                            steps.append(f"{step_number}. {step_text}")
            
            if steps:
                instructions = "\n".join(steps)
        
        # Nếu không có, thử lấy từ instructions (HTML)
        if not instructions and 'instructions' in spoonacular_data:
            raw_instructions = spoonacular_data['instructions']
            if raw_instructions:
                # Loại bỏ HTML tags đơn giản
                instructions = re.sub(r'<[^>]+>', '', raw_instructions)
                instructions = instructions.strip()
        
        # Nếu vẫn không có, tạo placeholder
        if not instructions:
            instructions = "Hướng dẫn nấu ăn sẽ được cập nhật sau."
        
        return instructions
    
    @staticmethod
    def print_debug(recipes: List[Recipe]): 
        print(f"Tìm thấy {len(recipes)} công thức phù hợp:")
        print("=" * 60)

        for i, recipe in enumerate(recipes, 1):
            print(f"\nCÔNG THỨC {i}: {recipe.title}")
            print(f"Thời gian nấu: {recipe.cook_time} phút")
            print(f"Phục vụ: {recipe.people_serving} người")
            print(f"Độ khó: {recipe.difficulty.upper()}")
            print(f"Hình ảnh: {recipe.image_url}")
            print(f"Hướng dẫn:\n{recipe.instructions[:200]}{'...' if len(recipe.instructions) > 200 else ''}")
            print("-" * 60)


# Ví dụ mẫu để test
if __name__ == "__main__":
    # Tạo danh sách nguyên liệu mẫu
    ingredients = [
        Food(
            id="food_001",
            name="chicken",
            quantity=500,
            unit="g",
            is_good=True,
            is_expired=False,
            input_date=datetime.now(),
            output_date=datetime.now(),
            category="meat"
        ),
        Food(
            id="food_002", 
            name="rice",
            quantity=200,
            unit="g",
            is_good=True,
            is_expired=False,
            input_date=datetime.now(),
            output_date=datetime.now(),
            category="grain"
        ),
        Food(
            id="food_003",
            name="onion",
            quantity=100,
            unit="g", 
            is_good=True,
            is_expired=False,
            input_date=datetime.now(),
            output_date=datetime.now(),
            category="vegetable"
        )
    ]
    
    # Lấy danh sách công thức
    recipes = GetRecipeService.get_list_recipe_by_spoonacular(
        ingredients=ingredients,
        diet="vegetarian",
        number=5
    )
    
    GetRecipeService.print_debug(recipes)
    