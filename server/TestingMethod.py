from thirdparty.database.model import Food, Condition
from thirdparty.database.method import add_food, get_all_foods, delete_food_by_id, delete_all_foods, add_fridge_conditions
from my_util import get_calo_usda, get_category, get_image_url
from datetime import datetime, timedelta

def test_add_food():
    now = datetime.now()
    
    foods = [
        Food(
                id="1", 
                name="táo", 
                quantity=10, 
                unit="quả",
                is_good=True, 
                is_expired=False,
                input_date=now, 
                output_date=now + timedelta(days=5), 
                category=get_category("táo"),
                image_url=get_image_url("táo"), 
                calo=get_calo_usda("táo", 10, "quả")
            ),

        Food(
                id="2", 
                name="cá hồi", 
                quantity=200, 
                unit="gam",
                is_good=True, 
                is_expired=False,
                input_date=now, 
                output_date=now + timedelta(days=3), 
                category=get_category("cá hồi"),
                image_url=get_image_url("cá hồi"), 
                calo=get_calo_usda("cá hồi", 200, "gam")
            ),

        Food(
                id="3", 
                name="trứng", 
                quantity=20, 
                unit="quả",
                is_good=True, 
                is_expired=False,
                input_date=now, 
                output_date=now + timedelta(days=4), 
                category=get_category("trứng"),
                image_url=get_image_url("trứng"), 
                calo=get_calo_usda("trứng", 20, "quả")
            ),
    ]

    for food in foods:
        add_food(food)

def delete_all_foods_(): 
    delete_all_foods()

def test_get_all_foods():
    foods = get_all_foods()
    return foods

def test_delete_food_by_id():
    delete_food_by_id("2")  # Xóa chuối

def food_all_test():
    delete_all_foods_()
    test_add_food()
    test_get_all_foods()

def add_condition(): 
    now = datetime.now()
    sample_condition = Condition("1", now, 0.0, 0.0, False, 0, "1")
    add_fridge_conditions(sample_condition)

if __name__ == "__main__":
    food_all_test()