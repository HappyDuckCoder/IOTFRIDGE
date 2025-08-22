# connect firebase 

# file configuration thì copy vào file configuration.json

import firebase_admin
import os
from firebase_admin import credentials, firestore

def get_firestore_db():
    # Nạp thông tin xác thực từ file JSON
    current_dir = os.path.dirname(os.path.abspath(__file__))
    cred_path = os.path.join(current_dir, "configuration.json")
    cred = credentials.Certificate(cred_path)
    # Khởi tạo Firebase nếu chưa có
    if not firebase_admin._apps:
        firebase_admin.initialize_app(cred)
    # Trả về đối tượng db
    return firestore.client()

def main():
    db = get_firestore_db()
    print("Firebase Firestore đã được kết nối thành công!")
    # Thử truy vấn một collection
    docs = db.collection('Food').get()
    for doc in docs:
        print(f'{doc.id}: {doc.to_dict()}')

if __name__ == "__main__":
    main()