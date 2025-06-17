# những method bên model 

from model import Food
from connect import get_firestore_db

db = get_firestore_db()
collection_name = "Food"

def add_food(food: Food):
    doc_ref = db.collection(collection_name).document(food.id)
    doc = doc_ref.get()

    if doc.exists:
        # Nếu đã có thì tăng số lượng
        existing_data = doc.to_dict()
        new_quantity = existing_data.get("quantity", 0) + food.quantity
        doc_ref.update({"quantity": new_quantity})
        print(f"Đã cập nhật số lượng {food.name} lên {new_quantity} {food.unit}.")
    else:
        # Nếu chưa có thì thêm mới
        doc_ref.set(food.to_dict())
        print(f"Đã thêm món ăn mới: {food.name}")

# Lấy toàn bộ danh sách đồ ăn
def get_all_foods():
    docs = db.collection(collection_name).stream()
    return [Food.from_dict(doc.to_dict()) for doc in docs]

# Xóa một loại đồ ăn
def delete_food(food_id):
    db.collection(collection_name).document(food_id).delete()

# xóa đồ ăn theo id

# Tìm kiếm theo trạng thái
def get_foods_by_status(status: str):
    docs = db.collection(collection_name).where("status", "==", status).stream()
    return [Food.from_dict(doc.to_dict()) for doc in docs]
