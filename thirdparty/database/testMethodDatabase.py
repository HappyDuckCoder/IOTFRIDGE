# testMethodDatabase.py

from model import Food
from method import add_food, get_all_foods, delete_food

def test():
    print("\n\U0001F4E6 Bắt đầu test thêm và in dữ liệu từ tủ lạnh:\n")

    # Thêm đồ ăn mới
    A = Food(id="1", name="Táo", quantity=10, unit="quả")
    add_food(A)

    B = Food(id="2", name="Chuối", quantity=5, unit="quả")
    add_food(B)

    C = Food(id="3", name="Cam", quantity=8, unit="quả")
    add_food(C)

    D = Food(id="4", name="Thịt", quantity=100, unit="gam")
    add_food(D)

    # In toàn bộ đồ ăn trong tủ lạnh
    foods = get_all_foods()
    print("\n\U0001F4DD Danh sách món ăn trong tủ:")
    for f in foods:
        print(f"- {f.name}: {f.quantity} {f.unit}")

def main():
    print("\n\U0001F680 Chạy test chính:")
    A = Food(id="5", name="Thịt gà", quantity=1, unit="miếng")
    add_food(A)
    test()

if __name__ == "__main__":
    main()