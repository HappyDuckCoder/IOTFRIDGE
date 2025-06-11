# test code here

from email.utils import formataddr
from model import Food
from method import add_food, get_all_foods

# Thêm đồ ăn mới
new_food = Food(id="banana1", name="Chuối", quantity=3, unit="trái", status="còn tươi")
add_food(new_food)
food_1 = Food(id="apple1", name="Táo", quantity=5, unit="trái", status="còn tươi")
add_food(food_1)
food_2 = Food(id="orange1", name="Cam", quantity=2, unit="trái", status="bị hư")
add_food(food_2)

# In toàn bộ đồ ăn
foods = get_all_foods()
for f in foods:
    print(f"{f.name}: {f.quantity} {f.unit} \nTrạng thái: {f.status}")