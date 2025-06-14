# test code here

from email.utils import formataddr
from re import A
from model import Food
from method import add_food, get_all_foods, delete_food, get_foods_by_status

# Thêm đồ ăn mới
A = Food(id="1", name="Táo", quantity=10, unit="quả", good=10, expired=0)
add_food(A)
B = Food(id="2", name="Chuối", quantity=5, unit="quả", good=3, expired=2)
add_food(B)
C = Food(id="3", name="Cam", quantity=8, unit="quả", good=6, expired=2)
add_food(C)
D = Food(id="4", name="Thịt", quantity=100, unit="gam", good="tươi", expired="")
add_food(D)

# In toàn bộ đồ ăn
foods = get_all_foods()
for f in foods:
    print(f"{f.name}: {f.quantity} {f.unit} \nTrạng thái: {f.good} {f.expired}")