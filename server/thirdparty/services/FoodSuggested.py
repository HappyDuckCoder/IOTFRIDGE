from thirdparty.database.model import Food
import numpy as np
from datetime import datetime

class FoodSuggestedService:
    def __init__(self):
        pass

    # hàm định nghĩa các tham số weight của hàm heuristic
    @staticmethod
    def heuristic_preparation():
        # trả về một list các tham số a, b, c
        t = (0.3, 0.2, 0.5)          # trọng số a, b, c  nutrition, preference, freshness
        return t

    # tính độ fit của món ăn dựa trên category và diet type
    @staticmethod
    def calculate_nutrition_fitness(food: Food, diet="healthy"):
        base_score = 0.5
        category = food.category.lower()
        
        # Điểm cơ sở cho từng loại thực phẩm
        category_scores = {
            "vegetable_and_vitamins": 1.0,  # Rau củ và vitamin - tốt nhất
            "protein": 0.8,                 # Protein - tốt
            "carbohydrate": 0.6,           # Carbohydrate - trung bình
            "fat": 0.4                     # Chất béo - thấp nhất
        }
        
        base_score = category_scores.get(category, 0.5)
        
        # Điều chỉnh theo từng loại diet
        diet_adjustments = {
            "healthy": {
                "vegetable_and_vitamins": 0.3,
                "protein": 0.2,
                "carbohydrate": 0.0,
                "fat": -0.2
            },
            "low-calories": {
                "vegetable_and_vitamins": 0.4,
                "protein": 0.1,
                "carbohydrate": -0.2,
                "fat": -0.3
            },
            "high-calories": {
                "vegetable_and_vitamins": -0.1,
                "protein": 0.3,
                "carbohydrate": 0.2,
                "fat": 0.4
            },
            "vegetarian": {
                "vegetable_and_vitamins": 0.4,
                "protein": -0.1,  # Protein động vật sẽ bị điều chỉnh riêng
                "carbohydrate": 0.2,
                "fat": 0.0
            },
            "keto": {
                "vegetable_and_vitamins": 0.1,
                "protein": 0.3,
                "carbohydrate": -0.4,
                "fat": 0.5
            },
            "clean-eating": {
                "vegetable_and_vitamins": 0.4,
                "protein": 0.2,
                "carbohydrate": -0.1,
                "fat": -0.2
            }
        }
        
        if diet in diet_adjustments:
            adjustment = diet_adjustments[diet].get(category, 0)
            base_score = max(0.1, min(1.0, base_score + adjustment))

        # Điều chỉnh dựa trên calories
        if hasattr(food, 'calo') and food.calo:
            if diet == "low-calories" and food.calo > 300:
                base_score *= 0.7
            elif diet == "high-calories" and food.calo < 200:
                base_score *= 0.8

        # Penalty nếu thực phẩm không tốt hoặc hết hạn
        if food.is_expired:
            base_score = 0.0
        elif not food.is_good:
            base_score *= 0.2
            
        return base_score
    
    # tính điểm freshness - độ tươi của thực phẩm
    @staticmethod
    def calculate_freshness_score(food: Food):
        if food.is_expired:
            return 0.0
        if not food.is_good:
            return 0.1
            
        # Tính số ngày còn lại đến hạn sử dụng
        today = datetime.now()

        if food.output_date:
            days_until_expiry = (food.output_date - today).days
            if days_until_expiry <= 0:
                return 0.0
            elif days_until_expiry <= 1:
                return 1.0  # Sắp hết hạn - nên dùng ngay (điểm cao để ưu tiên)
            elif days_until_expiry <= 2:
                return 0.9
            elif days_until_expiry <= 3:
                return 0.8
            elif days_until_expiry <= 7:
                return 0.7
            elif days_until_expiry <= 14:
                return 0.6
            else:
                return 0.5  # Còn lâu hạn sử dụng
        else:
            return 0.4  # Không có thông tin hạn sử dụng

    # tính điểm ưu tiên của món ăn dựa trên sở thích người dùng
    @staticmethod
    def calculate_user_preference(food: Food):
        # food.user_preference là điểm đánh giá từ người dùng (0-10)
        base_score = 0.0
        base_score += food.user_preference / 10.0

        # Penalty nếu thực phẩm không tốt hoặc hết hạn
        if food.is_expired:
            base_score = 0.0
        elif not food.is_good:
            base_score *= 0.2
            
        return base_score

    # hàm heuristic để xác định món ăn ưu tiên
    # Heuristic Score = a ×  NutritionFitness + b × UserPreference + c × FreshnessScore
    @staticmethod
    def heuristic_priority(t, food: Food, diet="healthy"):
        nutrition_fitness = FoodSuggestedService.calculate_nutrition_fitness(food, diet)
        user_preference = FoodSuggestedService.calculate_user_preference(food)
        freshness_score = FoodSuggestedService.calculate_freshness_score(food)
        a, b, c  = t
        score =  a * nutrition_fitness + b * user_preference + c * freshness_score
        
        return score

    @staticmethod
    def get_top_food_suggestions(foods, diet="healthy", top_n=5):
        """Lấy top N món ăn được gợi ý cao nhất"""
        t = FoodSuggestedService.heuristic_preparation()
        food_scores = []
        
        for food in foods:
            # Bỏ qua thực phẩm đã hết hạn
            if food.is_expired:
                continue
                
            score = FoodSuggestedService.heuristic_priority(t, food, diet)
            food_scores.append((food, score))
        
        # Sắp xếp theo điểm số giảm dần
        food_scores.sort(key=lambda x: x[1], reverse=True)
        
        return food_scores[:top_n]
    
    @staticmethod
    def debug(food, t, diet="healthy"):
        nutrition_fit = FoodSuggestedService.calculate_nutrition_fitness(food, diet)
        user_pref = FoodSuggestedService.calculate_user_preference(food)
        freshness = FoodSuggestedService.calculate_freshness_score(food)
        total_score = FoodSuggestedService.heuristic_priority(t, food, diet)
        
        print(f"Thực phẩm: {food.name} ({food.category})")
        print(f"  Nutrition Fitness: {nutrition_fit:.3f}")
        print(f"  User Preference: {user_pref:.3f}")
        print(f"  Freshness Score: {freshness:.3f}")
        print(f"  Tổng điểm: {total_score:.3f}")
        print()