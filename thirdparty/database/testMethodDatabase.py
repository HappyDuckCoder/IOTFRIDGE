from model import Food
from method import add_food, get_all_foods, delete_food_by_id

from datetime import datetime, timedelta

def get_image_url(food_name):
    return f"https://example.com/images/{food_name}.jpg" # lấy của đoàn

def get_category(food_name):
    # 1 prompt: "vitamin_and_fruits", "protein", "carbohydrate", "fat"

    return "vitamin_and_fruits"

def test_add_food():
    now = datetime.now()
    
    foods = [
        Food(id="1", name="Táo", quantity=10, unit="quả",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=5), category="vitamin_and_fruits",
             image_url=None),

        Food(id="2", name="Chuối", quantity=5, unit="quả",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=3), category="vitamin_and_fruits",
             image_url=None),

        Food(id="3", name="Cam", quantity=8, unit="quả",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=4), category="vitamin_and_fruits",
             image_url=None),

        Food(id="4", name="Thịt", quantity=100, unit="gam",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=2), category="protein",
             image_url=None),
    ]

    for f in foods:
        add_food(f)

def test_get_all_foods():
    foods = get_all_foods()
    for f in foods:
        print(f"- {f.name}: {f.quantity} {f.unit}")
        print(f"  Hạn dùng: {f.output_date.strftime('%Y-%m-%d')}")
        print(f"  Còn tốt? {'Có' if f.is_good else 'Không'} | Hết hạn? {'Có' if f.is_expired else 'Chưa'}\n")

def test_delete_food_by_id():
    delete_food_by_id("2")  # Xóa chuối

def main():
    test_add_food()
    test_get_all_foods()
    test_delete_food_by_id()

if __name__ == "__main__":
    main()
