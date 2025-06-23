# thirdparty/database/method.py

from connect import db
from model import Food
from notification import send_to_topic

collection_name = "foods"

def add_food(food: Food):
    doc_ref = db.collection(collection_name).document(food.id)
    doc = doc_ref.get()

    if doc.exists:
        existing_data = doc.to_dict()
        new_quantity = existing_data.get("quantity", 0) + food.quantity
        doc_ref.update({"quantity": new_quantity})
        print(f"\u2705 Cập nhật {food.name}: {new_quantity} {food.unit}")

        # Gửi thông báo nếu sắp hết
        if new_quantity <= 1:
            send_to_topic(
                topic="fridge_alerts",
                title="Cảnh báo tủ lạnh",
                body=f"{food.name} sắp hết! Chỉ còn {new_quantity} {food.unit}."
            )
    else:
        doc_ref.set(food.to_dict())
        print(f"\u2795 Đã thêm món mới: {food.name}")

        # Gửi thông báo khi thêm mới
        send_to_topic(
            topic="fridge_alerts",
            title="Tủ lạnh cập nhật",
            body=f"Đã thêm {food.quantity} {food.unit} {food.name} vào tủ."
        )

def get_all_foods():
    foods_ref = db.collection(collection_name).stream()
    foods = []
    for doc in foods_ref:
        data = doc.to_dict()
        data["id"] = doc.id  # thêm id vào dict
        foods.append(Food(**data))  # truyền duy nhất 1 dict
    return foods

def update_food(food_id, update_data: dict):
    db.collection(collection_name).document(food_id).update(update_data)
    print(f"\u2705 Đã cập nhật {food_id}")

def delete_food(food_id):
    db.collection(collection_name).document(food_id).delete()
    print(f"\u274C Đã xoá món ăn: {food_id}")