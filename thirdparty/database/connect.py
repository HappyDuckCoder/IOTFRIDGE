# connect firebase 

# file configuration thì copy vào file configuration.json


import firebase_admin
from firebase_admin import credentials, firestore

def get_firestore_db():
    cred = credentials.Certificate("configuration.json")
    firebase_admin.initialize_app(cred)
    return firestore.client()

# Khởi tạo sẵn 1 biến db dùng chung
db = get_firestore_db()