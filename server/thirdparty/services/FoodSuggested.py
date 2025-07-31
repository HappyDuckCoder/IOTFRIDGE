from thirdparty.database.model import Food
import numpy as np
from datetime import datetime


class FoodSuggestedService:
    def __init__(self):
        pass

    # hàm định nghĩa các tham số weight của hàm heuristic
    @staticmethod
    def heuristic_preparation():
        # trả về một list các trọng số [w], một list các score đặc trưng [u], một list các tham số α, β, γ 
        w = np.array([1.0, 0.8, 1.2, 0.5, 1.0, 0.7, 0.6])  # trọng số cho các thành phần
        u = np.array([1, 0.9, 0.7, 1, 0.85, 0.6, 0.9])     # score đặc trưng của món ăn
        t = (0.4, 0.3, 0.3, 0.1)                                # trọng số α, β, γ, δ
        return w, u, t

    # CosineSimilarity(u,i) = u ⋅ i / ∥u∥ ⋅ ∥i∥ 
    @staticmethod
    def calculate_cosine_similarity(w, u): 
        numerator = np.dot(w, u)
        denominator = np.linalg.norm(w) * np.linalg.norm(u)
        return numerator / denominator if denominator != 0 else 0

    # tính độ fit của món ăn dựa trên category và freshness
    @staticmethod
    def calculate_nutrition_fitness(food: Food, diet="healthy"):
        base_score = 0.5
        
        # Điều chỉnh điểm dựa trên category
        healthy_categories = ["vegetables", "fruits", "lean_protein", "whole_grains"]
        moderate_categories = ["dairy", "nuts", "fish", "chicken"]
        low_categories = ["processed", "sweets", "fast_food", "fried"]
        
        if food.category.lower() in healthy_categories:
            base_score = 1.0
        elif food.category.lower() in moderate_categories:
            base_score = 0.7
        elif food.category.lower() in low_categories:
            base_score = 0.3
            
        # Điều chỉnh theo diet cụ thể
        if diet == "low-calories":
            if food.category.lower() in ["vegetables", "fruits"]:
                base_score = min(1.0, base_score + 0.2)
            elif food.category.lower() in ["sweets", "fried"]:
                base_score = max(0.1, base_score - 0.3)
                
        elif diet == "high-calories":
            if food.category.lower() in ["nuts", "dairy", "lean_protein"]:
                base_score = min(1.0, base_score + 0.2)
            elif food.category.lower() in ["vegetables"]:
                base_score = max(0.3, base_score - 0.2)
                
        elif diet == "vegetarian":
            if food.category.lower() in ["vegetables", "fruits", "dairy", "nuts"]:
                base_score = min(1.0, base_score + 0.3)
            elif food.category.lower() in ["lean_protein", "fish", "chicken"]:
                base_score = 0.1
                
        elif diet == "clean-eating":
            if food.category.lower() in ["vegetables", "fruits", "whole_grains"]:
                base_score = min(1.0, base_score + 0.3)
            elif food.category.lower() in ["processed", "fast_food"]:
                base_score = 0.1

        # Penalty nếu thực phẩm không tốt hoặc hết hạn
        if food.is_expired:
            base_score = 0.0
        elif not food.is_good:
            base_score *= 0.3
            
        return base_score
    
    # tính điểm freshness - độ tươi của thực phẩm
    @staticmethod
    def calculate_freshness_score(food: Food):
        if food.is_expired:
            return 0.0
        if not food.is_good:
            return 0.2
            
        # Tính số ngày còn lại đến hạn sử dụng
        today = datetime.now()

        if food.output_date:
            days_until_expiry = (food.output_date - today).days
            if days_until_expiry <= 0:
                return 0.0
            elif days_until_expiry <= 1:
                return 0.9  # Sắp hết hạn - nên dùng ngay
            elif days_until_expiry <= 3:
                return 0.8
            elif days_until_expiry <= 7:
                return 0.7
            else:
                return 0.6
        else:
            return 0.5  # Không có thông tin hạn sử dụng

    # tính điểm ưu tiên của món ăn dựa trên sở thích người dùng
    @staticmethod
    def calculate_user_preference(food: Food):
        # food.user_preference là điểm đánh giá từ người dùng (1-10)
        score = food.user_preference / 10.0  # Normalize về 0-1
        
        # Bonus nếu là món ăn ưu tiên
        if food.is_priority_food:
            score = min(1.0, score + 0.2)
            
        return score

    # hàm heuristic để xác định món ăn ưu tiên
    # Heuristic Score = α × CosineSimilarity + β × NutritionFitness + γ × UserPreference + δ × FreshnessScore
    @staticmethod
    def heuristic_priority(w, u, t, food: Food, diet="healthy"):
        cosine_similarity = FoodSuggestedService.calculate_cosine_similarity(w, u)
        nutrition_fitness = FoodSuggestedService.calculate_nutrition_fitness(food, diet)
        user_preference = FoodSuggestedService.calculate_user_preference(food)
        freshness_score = FoodSuggestedService.calculate_freshness_score(food)
        
        # α, β, γ, δ là các trọng số cho từng thành phần
        alpha, beta, gamma, delta = t
        
        score = (alpha * cosine_similarity + 
                beta * nutrition_fitness + 
                gamma * user_preference + 
                delta * freshness_score)
        
        return score

    @staticmethod
    def get_top_food_suggestions(foods, diet="healthy", top_n=5):
        """Lấy top N món ăn được gợi ý cao nhất"""
        w, u, t = FoodSuggestedService.heuristic_preparation()
        food_scores = []
        
        for food in foods:
            # Bỏ qua thực phẩm đã hết hạn
            if food.is_expired:
                continue
                
            score = FoodSuggestedService.heuristic_priority(w, u, t, food, diet)
            food_scores.append((food, score))
        
        # Sắp xếp theo điểm số giảm dần
        food_scores.sort(key=lambda x: x[1], reverse=True)
        
        return food_scores[:top_n]
    
    @staticmethod
    def debug(food, w, u, t):

        cosine_sim = FoodSuggestedService.calculate_cosine_similarity(w, u)
        nutrition_fit = FoodSuggestedService.calculate_nutrition_fitness(food, "healthy")
        user_pref = FoodSuggestedService.calculate_user_preference(food)
        freshness = FoodSuggestedService.calculate_freshness_score(food)
        total_score = FoodSuggestedService.heuristic_priority(w, u, t, food, "healthy")
        
        print(f"Thực phẩm: {food.name}")
        print(f"  Cosine Similarity: {cosine_sim:.3f}")
        print(f"  Nutrition Fitness: {nutrition_fit:.3f}")
        print(f"  User Preference: {user_pref:.3f}")
        print(f"  Freshness Score: {freshness:.3f}")
        print(f"  Tổng điểm: {total_score:.3f}")
        print()

def main():
    foods = [
        Food(name="Món 1", category="healthy", is_priority_food=True, is_good=True),
        Food(name="Món 2", category="healthy", is_priority_food=False, is_good=True),
        Food(name="Món 3", category="healthy", is_priority_food=False, is_good=True),
    ]    
    # Test với các loại diet khác nhau
    list_diets = ["healthy", "vegetarian", "low-calories", "high-calories", "clean-eating"]
    
    top_suggestions = FoodSuggestedService.get_top_food_suggestions(foods, "vegetarian", top_n=3)
    print("Top 3 gợi ý:")
    for i, (food, score) in enumerate(top_suggestions, 1):
        print(f"  {i}. {food.name} - Điểm: {score:.3f} ({food.category})")
    print()    
    
    w, u, t = FoodSuggestedService.heuristic_preparation()

    for food in foods[:3]:  # Test 3 món đầu tiên
        if food.is_expired:
            continue

        FoodSuggestedService.debug(food, w, u, t)
        
if __name__ == "__main__":
    main()