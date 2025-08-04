from thirdparty.database.model import Food
import numpy as np
from datetime import datetime

class FoodSuggestedService:
    def __init__(self):
        pass

    # hàm định nghĩa các tham số weight của hàm heuristic
    @staticmethod
    def heuristic_preparation():
        # trả về một list các trọng số [w], một list các score đặc trưng [u], một list các tham số α, β, γ, δ
        w = np.array([1.0, 0.8, 1.2, 0.9])  # trọng số cho 4 categories: fat, protein, carbohydrate, vegetable_and_vitamins
        u = np.array([0.7, 1.0, 0.8, 1.0])  # score đặc trưng tương ứng với từng category
        t = (0.2, 0.35, 0.25, 0.2)          # trọng số α, β, γ, δ cho cosine, nutrition, preference, freshness
        return w, u, t

    # CosineSimilarity(u,i) = u ⋅ i / ∥u∥ ⋅ ∥i∥ 
    @staticmethod
    def calculate_cosine_similarity(w, u): 
        numerator = np.dot(w, u)
        denominator = np.linalg.norm(w) * np.linalg.norm(u)
        return numerator / denominator if denominator != 0 else 0

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
        base_score = food.user_preference / 10.0  # Normalize về 0-1
        
        # Bonus nếu là món ăn ưu tiên
        if hasattr(food, 'is_priority_food') and food.is_priority_food:
            base_score = min(1.0, base_score + 0.3)
        
        # Điều chỉnh dựa trên quantity - ưu tiên món có ít quantity hơn để tránh hỏng
        if hasattr(food, 'quantity') and food.quantity:
            if food.quantity < 1:  # Còn ít
                base_score = min(1.0, base_score + 0.1)
            elif food.quantity > 5:  # Còn nhiều
                base_score = max(0.1, base_score - 0.05)
            
        return base_score

    # tính điểm category vector dựa trên category của food
    @staticmethod
    def get_category_vector(category: str):
        """Chuyển đổi category thành vector để tính cosine similarity"""
        category_vectors = {
            "fat": np.array([1, 0, 0, 0]),
            "protein": np.array([0, 1, 0, 0]),
            "carbohydrate": np.array([0, 0, 1, 0]),
            "vegetable_and_vitamins": np.array([0, 0, 0, 1])
        }
        return category_vectors.get(category.lower(), np.array([0.25, 0.25, 0.25, 0.25]))

    # hàm heuristic để xác định món ăn ưu tiên
    # Heuristic Score = α × CosineSimilarity + β × NutritionFitness + γ × UserPreference + δ × FreshnessScore
    @staticmethod
    def heuristic_priority(w, u, t, food: Food, diet="healthy"):
        # Sử dụng category vector thay vì u cố định
        food_vector = FoodSuggestedService.get_category_vector(food.category)
        cosine_similarity = FoodSuggestedService.calculate_cosine_similarity(w, food_vector)
        
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
    def debug(food, w, u, t, diet="healthy"):
        food_vector = FoodSuggestedService.get_category_vector(food.category)
        cosine_sim = FoodSuggestedService.calculate_cosine_similarity(w, food_vector)
        nutrition_fit = FoodSuggestedService.calculate_nutrition_fitness(food, diet)
        user_pref = FoodSuggestedService.calculate_user_preference(food)
        freshness = FoodSuggestedService.calculate_freshness_score(food)
        total_score = FoodSuggestedService.heuristic_priority(w, u, t, food, diet)
        
        print(f"Thực phẩm: {food.name} ({food.category})")
        print(f"  Category Vector: {food_vector}")
        print(f"  Cosine Similarity: {cosine_sim:.3f}")
        print(f"  Nutrition Fitness: {nutrition_fit:.3f}")
        print(f"  User Preference: {user_pref:.3f}")
        print(f"  Freshness Score: {freshness:.3f}")
        print(f"  Tổng điểm: {total_score:.3f}")
        print()

def main():
    # Tạo test data với các category mới
    foods = [
        Food(
            id="1", name="Thịt bò", quantity=0.5, unit="kg",
            is_good=True, is_expired=False,
            input_date=datetime.now(), output_date=datetime(2025, 8, 10),
            category="protein", calo=250, user_preference=8, is_priority_food=True
        ),
        Food(
            id="2", name="Rau cải", quantity=2, unit="bó",
            is_good=True, is_expired=False,
            input_date=datetime.now(), output_date=datetime(2025, 8, 6),
            category="vegetable_and_vitamins", calo=25, user_preference=6
        ),
        Food(
            id="3", name="Gạo", quantity=3, unit="kg",
            is_good=True, is_expired=False,
            input_date=datetime.now(), output_date=datetime(2025, 12, 1),
            category="carbohydrate", calo=130, user_preference=7
        ),
        Food(
            id="4", name="Dầu olive", quantity=0.8, unit="chai",
            is_good=True, is_expired=False,
            input_date=datetime.now(), output_date=datetime(2025, 10, 15),
            category="fat", calo=884, user_preference=5
        )
    ]
    
    # Test với các loại diet khác nhau
    list_diets = ["healthy", "vegetarian", "low-calories", "high-calories", "keto", "clean-eating"]
    
    for diet in list_diets:
        print(f"\n=== DIET: {diet.upper()} ===")
        top_suggestions = FoodSuggestedService.get_top_food_suggestions(foods, diet, top_n=3)
        print("Top 3 gợi ý:")
        for i, (food, score) in enumerate(top_suggestions, 1):
            print(f"  {i}. {food.name} - Điểm: {score:.3f} ({food.category}, {food.calo} calo)")
    
    print("\n=== DEBUG CHI TIẾT ===")
    w, u, t = FoodSuggestedService.heuristic_preparation()
    for food in foods:
        if food.is_expired:
            continue
        FoodSuggestedService.debug(food, w, u, t, "healthy")
        
if __name__ == "__main__":
    main()