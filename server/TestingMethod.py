from thirdparty.database.model import Food, Condition, Setting
from thirdparty.api.GetCalo import GetCaloService
from thirdparty.database.method import add_food, get_all_foods, delete_food_by_id, delete_all_foods, add_fridge_conditions, add_setting, delete_setting_by_id, get_setting_by_id, update_setting_by_id
from my_util import get_category, get_image_url
from datetime import datetime, timedelta

def get_calo_usda(food_name, quantity, unit): 
    service = GetCaloService()
    return service.get_calo_with_quantity(food_name, quantity, unit)

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
    for food in foods:
        print(food.to_dict())

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

def test_setting():
    sample_setting = Setting(
        id="2", 
        diet="healthy",
        fan_mode=1,
        is_open_notification=False,
        fridge_data_id="1"
    )

    # Thêm setting
    add_setting(sample_setting)
    print("Đã thêm setting.")

    # Cập nhật fan_mode
    update_setting_by_id("2", {"fan_mode": 0})
    print("Đã cập nhật fan_mode.")

    # Lấy lại để kiểm tra
    get_setting = get_setting_by_id("2")
    print("Dữ liệu sau cập nhật:", get_setting.to_dict() if get_setting else "Không tìm thấy")

    # Xoá
    delete_setting_by_id("2")
    print("Đã xoá setting.")

    # Kiểm tra lại sau xoá
    get_setting = get_setting_by_id("2")
    print("Kết quả sau khi xoá:", get_setting if get_setting else "Không tồn tại")


if __name__ == "__main__":
    test_setting()