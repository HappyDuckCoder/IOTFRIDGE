import requests
from keys import *

message = input("Enter message: ")

url = f"https://api.telegram.org/bot{bot_token}/sendMessage"
payload = {"chat_id": chat_id, "text": message}

r = requests.post(url, data=payload)

print(r.json())
