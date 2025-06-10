# những method bên model 

# CRUD (CREATE

from model import Food
from connect import get_firestore_db

db = get_firestore_db()
collection_name = "foods"

# Thêm đồ ăn vào database
def add_food(food: Food):
    db.collection(collection_name).document(food.id).set(food.to_dict())

# Lấy toàn bộ danh sách đồ ăn
def get_all_foods():
    docs = db.collection(collection_name).stream()
    return [Food.from_dict(doc.to_dict()) for doc in docs]

# Cập nhật một loại đồ ăn
def update_food(food_id, update_data: dict):
    db.collection(collection_name).document(food_id).update(update_data)

# Xóa một loại đồ ăn
def delete_food(food_id):
    db.collection(collection_name).document(food_id).delete()

# Tìm kiếm theo trạng thái
def get_foods_by_status(status: str):
    docs = db.collection(collection_name).where("status", "==", status).stream()
    return [Food.from_dict(doc.to_dict()) for doc in docs]
