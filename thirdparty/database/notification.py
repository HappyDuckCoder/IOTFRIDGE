# thirdparty/database/notification.py

import firebase_admin
from firebase_admin import credentials, messaging

# Khởi tạo Firebase (nếu chưa)
def init_firebase():
    if not firebase_admin._apps:
        cred = credentials.Certificate("configuration.json")
        firebase_admin.initialize_app(cred)

# Gửi thông báo đến 1 thiết bị (qua FCM token)
def send_notification(token, title, body):
    init_firebase()

    message = messaging.Message(
        notification=messaging.Notification(
            title=title,
            body=body,
        ),
        token=token
    )

    response = messaging.send(message)
    print("Đã gửi thông báo:", response)

# Gửi thông báo đến topic (không cần token)
def send_to_topic(topic, title, body):
    init_firebase()

    message = messaging.Message(
        notification=messaging.Notification(
            title=title,
            body=body,
        ),
        topic=topic
    )

    response = messaging.send(message)
    print("Đã gửi thông báo đến topic:", response)