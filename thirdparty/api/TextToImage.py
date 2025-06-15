#input: food: trứng, thịt, rau,...

#output: link image (10 link)

import requests

url = "https://modelslab.com/api/v6/realtime/text2img" # nền tảng API

text = "something"

payload = {
    "key": "N9I0rKot6x3esFmjWwKaAqvyNfhfBYgzwtQlBX0uOgz6jTpzT13UA8IMJ01Y",  # thay thế bằng API tài khoản cá nhân khi tạo acc
    "prompt": f"a realistic photo of {text}", # muốn promt bao nhiêu thì quăng vô bấy nhiêu nha
    "negative_prompt": "", #Những gì không muốn xuất hiện trong ảnh
    "width": "512",      #size               
    "height": "512",  
    "samples": "10",     # số lượng ảnh muốn tạo
    "num_inference_steps": "20",   #Số bước khuếch tán (diffusion steps) Số càng cao thì ảnh càng chi tiết, nhưng sinh ảnh lâu hơn.
    "guidance_scale": 7.5, #Hệ số điều chỉnh độ “tuân thủ” prompt Giá trị từ 1 → 20. Càng cao thì mô hình càng bám sát prompt, nhưng dễ làm ảnh cứng hoặc không tự nhiên
    "webhook": None, #URL webhook để nhận kết quả sau khi ảnh được sinh
    "track_id": None #ID tùy chọn để bạn theo dõi tác vụ
}

headers = {
    "Content-Type": "application/json"
}

response = requests.post(url, headers=headers, json=payload)

try:
    result = response.json()
    if result.get("output"):
        print("Link:", result["output"][0])
    else:
        print("Không có ảnh. Phản hồi:", result)
except Exception as e:
    print("Lỗi JSON:", e)
    print("Phản hồi gốc:", response.text)