from thirdparty.database.model import Food
from thirdparty.database.method import add_food, get_all_foods, delete_food_by_id
from util import get_calo_usda, get_category

from datetime import datetime, timedelta

def test_add_food():
    now = datetime.now()
    
    foods = [
        Food(id="1", name="táo", quantity=10, unit="quả",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=5), category="vitamin_and_fruits",
             image_url=None, calo = 0),

        Food(id="2", name="cá hồi", quantity=200, unit="gam",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=3), category="vitamin_and_fruits",
             image_url=None, calo = 0),

        Food(id="3", name="trứng", quantity=20, unit="quả",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=4), category="vitamin_and_fruits",
             image_url=None, calo = 0),

        Food(id="4", name="thịt gà", quantity=100, unit="gam",
             is_good=True, is_expired=False,
             input_date=now, output_date=now + timedelta(days=2), category="protein",
             image_url=None, calo = 0),
    ]

    for f in foods:
        print(f"Đang xử lý: {f.name}")
        
        # DEBUG: Kiểm tra giá trị trả về
        calo_value = get_calo_usda(f.name)
        print(f"Calo từ API: {calo_value}")
        f.calo = calo_value
        
        category_value = get_category(f.name)
        print(f"Category từ API: {category_value}")
        f.category = category_value
        
        print(f"Trước khi add_food: calo = {f.calo}")
        add_food(f)
        print("="*50)

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

if __name__ == "__main__":
    main()
