# những method bên model

from model import Food
from connect import get_firestore_db

db = get_firestore_db()
collection_name = "Food"

# Thêm hoặc cập nhật món ăn
def add_food(food: Food):
    doc_ref = db.collection(collection_name).document(food.id)
    doc = doc_ref.get()

    if doc.exists:
        existing_data = doc.to_dict()
        new_quantity = existing_data.get("quantity", 0) + food.quantity
        doc_ref.update({"quantity": new_quantity})
        print(f"Đã cập nhật số lượng {food.name} lên {new_quantity} {food.unit}.")
    else:
        doc_ref.set(food.to_dict())
        print(f"Đã thêm món ăn mới: {food.name}")

# Lấy toàn bộ danh sách
def get_all_foods():
    docs = db.collection(collection_name).stream()
    return [Food.from_dict(doc.to_dict()) for doc in docs]

# Lấy món ăn theo tên
def get_id_food_by_name(name):
    docs = db.collection(collection_name) \
             .where("name", "==", name) \
             .order_by("expiration_date") \
             .stream()
    
    return [(doc.id, doc.to_dict()["expiration_date"]) for doc in docs]

# Xóa theo ID
def delete_food_by_id(food_id):
    doc_ref = db.collection(collection_name).document(food_id)
    doc = doc_ref.get()

    if doc.exists:
        doc_ref.delete()
        print(f"Đã xóa thực phẩm có ID: {food_id}.")
    else:
        print(f"Không tìm thấy thực phẩm với ID: {food_id}.")

# Xóa toàn bộ
def delete_all_foods():
    docs = db.collection(collection_name).stream()
    for doc in docs:
        doc.reference.delete()

# Xóa theo tên
def eat_food(name, quantity):
    docs = db.collection(collection_name) \
             .where("name", "==", name) \
             .order_by("expiration_date") \
             .stream()

    remaining = quantity
    found = False

    for doc in docs:
        found = True
        data = doc.to_dict()
        current_quantity = data.get("quantity", 0)
        doc_ref = doc.reference

        if current_quantity >= remaining:
            doc_ref.update({"quantity": current_quantity - remaining})
            print(f"Đã ăn {quantity} {data['unit']} {name}.")
            return
        else:
            doc_ref.delete()
            print(f"Đã ăn hết {current_quantity} {data['unit']} từ lô {doc.id}.")
            remaining -= current_quantity

    if not found:
        print(f"Không tìm thấy món ăn tên '{name}' trong kho.")
    elif remaining > 0:
        print(f"Đã ăn {quantity - remaining} {data['unit']}, nhưng không đủ số lượng yêu cầu.")

# Tìm theo trạng thái (is_good hoặc is_expired)
def get_foods_by_status(field: str, value: bool):
    if field not in ["is_good", "is_expired"]:
        raise ValueError("Chỉ hỗ trợ tìm theo 'is_good' hoặc 'is_expired'")
    docs = db.collection(collection_name).where(field, "==", value).stream()
    return [Food.from_dict(doc.to_dict()) for doc in docs]
