import signal
import sys
import json
import socket
import threading
import time
import requests
from thirdparty.database.method import *
from thirdparty.api.SpeechToText import AudioModel
from thirdparty.services.handleTask import TaskHandling
from thirdparty.services.FoodSuggested import FoodSuggestedService
from thirdparty.api.GetRecipe import GetRecipeService
from thirdparty.services.notification import Notification
from my_util import *
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

TOP_FOOD = 3
TOP_RECIPE = 5
stt = AudioModel()
stt.load()

def get_len_food_default():
    return len(get_all_foods())

def add_new_recipes():    
    model_choose_ingredients = FoodSuggestedService()
    model_get_recipe = GetRecipeService()
    
    # lấy danh sách các môn ăn
    foods = get_all_foods()

    print("Danh sách các môn ăn:")
    for i, food in enumerate(foods, 1):
        print(f"  {i}. {food.name} ({food.category}) {food.input_date} - {food.output_date}")

    # lấy setting diet
    setting = get_setting_by_id("1")
    if setting:
        diet = setting.diet
    else:
        diet = "healthy"

    print("Diet hiện tại: ", diet)
     
    top_suggestions = model_choose_ingredients.get_top_food_suggestions(foods, diet, top_n=TOP_FOOD)
    
    print(f"Top {TOP_FOOD} gợi ý:")
    for i, (food, score) in enumerate(top_suggestions, 1):
        print(f"  {i}. {food.name} - Điểm: {score:.3f} ({food.category})")
    print() 

    ingredients = [translate_vietnamese_to_english(food.name) for food, _ in top_suggestions]

    recipes = model_get_recipe.get_list_recipe_by_spoonacular(
        ingredients=ingredients,
        diet=diet,
        number=TOP_RECIPE
    )

    model_get_recipe.print_debug(recipes)

    for recipe in recipes:
        add_recipe(recipe)

def monitor_food_changes():
    """Theo dõi thay đổi số lượng thực phẩm và cập nhật recipe khi cần"""
    old_len = get_len_food_default()
    print(f"Bắt đầu theo dõi thay đổi thực phẩm. Số lượng hiện tại: {old_len}")
    
    while True:
        try:
            time.sleep(5)  # Chờ 5 giây
            current_len = get_len_food_default()
            
            if current_len != old_len:
                print(f"Phát hiện thay đổi: {old_len} -> {current_len}")
                print("Đang cập nhật thực đơn mới...")
                add_new_recipes()
                old_len = current_len
                print("Cập nhật thực đơn hoàn thành!")
            else:
                print(f"Không có thay đổi thực phẩm (hiện tại: {current_len})")
                
        except Exception as e:
            print(f"Lỗi khi theo dõi thay đổi thực phẩm: {e}")
            time.sleep(5)  # Tiếp tục sau 5 giây nếu có lỗi

def get_setting_from_firebase():
    setting_data = get_setting_by_id(1)
    return setting_data

def send_test_data_to_esp(ip="192.168.1.1", port=8000):
    while True:
        try:
            test_data = {"test_data": 1}
            url = f"http://{ip}:{port}/receiveSetting"
            response = requests.post(url, json=test_data, timeout=5)
            print(f"Đã gửi setting tới ESP: {response.status_code}")
        except Exception as e:
            print(f"Lỗi khi gửi setting đến ESP: {e}")
        time.sleep(5)

def send_setting_to_esp(ip="192.168.1.1", port=8000):
    while True:
        try:
            setting_data = get_setting_from_firebase()
            url = f"http://{ip}:{port}/receiveSetting"
            response = requests.post(url, json=setting_data, timeout=5)
            print(f"Đã gửi setting tới ESP: {response.status_code}")
        except Exception as e:
            print(f"Lỗi khi gửi setting đến ESP: {e}")
        time.sleep(5)

def send_data_to_firebase(data):
    add_fridge_conditions(data)

def handle_add_food():
    # chuyển từ file pcm sang wav
    print("1. Chuyển file pcm sang wav")
    convert_pcm_to_wav("resources/mic.pcm", "resources/mic.wav")

    # speech to text resources/mic.pcm 
    print("2. Speech to text resources/mic.wav")
    text = stt.transcribe("resources/mic.wav")
    print(f"Nội dung từ speech-to-text: '{text}'")

    # tách các thành phần và phân loại hành động
    print("3. Tách thành phần câu lệnh")
    taskHandling = TaskHandling()
    component = taskHandling.DevideComponentInInput(text)

    if component is None:
        return "Lỗi khi thực hiện hàm DevideComponentInInput. Vui lòng thử laị."

    print("4. Phân loại hành động")
    classified_action = taskHandling.classifyAction(component.action)

    if classified_action is None:
        return "Không thể phân loại hành động."

    # thực hiện task
    print("5. Thực hiện thêm hoặc xóa")
    result = taskHandling.executeTask(component, classified_action)
    return result

class Handler(BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.file_name = "./resources/mic.pcm"
        self.file_name_text = "./resources/text.txt"
        Path("./resources").mkdir(exist_ok=True)
        super().__init__(*args, **kwargs)

    def get_local_ip():
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))  # IP bất kỳ, không cần truy cập được
            ip = s.getsockname()[0]
            s.close()
            return ip
        except Exception:
            return "127.0.0.1"

    # Nhận file âm thanh
    def post_audio(self): 
        try:
            content_length = int(self.headers['Content-Length'])
            audio_data = self.rfile.read(content_length)

            with open(self.file_name, 'wb') as f:
                f.write(audio_data)

            print("Đã nhận file ghi âm pcm từ ESP32")

            handle_add_food()

            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"Audio received successfully")

        except Exception as error:
            print(f"Lỗi trong post_audio: {error}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to receive audio")

    # Nhận dữ liệu test JSON
    def post_test_data(self):
        try:
            content_length = int(self.headers['Content-Length'])
            raw_data = self.rfile.read(content_length)
            json_data = json.loads(raw_data.decode('utf-8'))

            print("Dữ liệu test nhận từ ESP32:")
            print(json.dumps(json_data, indent=2))

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"status": "test received"}')

        except Exception as error:
            print(f"Lỗi trong post_test_data: {error}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to receive test data")

    # Nhận dữ liệu chính (ví dụ gửi lên Firebase sau này)
    def post_data(self):
        try:
            content_length = int(self.headers['Content-Length'])
            raw_data = self.rfile.read(content_length)
            json_data = json.loads(raw_data.decode('utf-8'))

            print("Dữ liệu thật nhận từ ESP32: ")
            print(json.dumps(json_data, indent=2))

            send_data_to_firebase(json_data)

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"status": "data received"}')

        except Exception as error:
            print(f"Lỗi trong post_data: {error}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to receive data")

    def post_notification(self):
        try:
            content_length = int(self.headers['Content-Length'])
            raw_data = self.rfile.read(content_length)
            json_data = json.loads(raw_data.decode('utf-8'))
            message = json_data.get("message", "Không có nội dung")

            print("Notification nhận từ ESP32:")
            print(message)

            Notification.sendMessage(message)

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"status": "message received"}')

        except Exception as error:
            print(f"Lỗi trong post_notification: {error}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to receive notification")


    def do_POST(self):
        if self.path == "/uploadAudio":
            self.post_audio()
        elif self.path == "/uploadData":
            self.post_data()
        elif self.path == "/uploadTestData":
            self.post_test_data()
        elif self.path == "/uploadNotification":
            self.post_notification()
        else:
            print("Error: Đường dẫn không hợp lệ")
            self.send_response(405)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"Method not allowed")

    def get_setting(self):
        try:
            condition_data = get_fridge_conditions_by_id("1")

            # Trả về JSON cấu hình
            response = {
                "temp": condition_data.temp,
                "humi": condition_data.humi,
                "is_rotted_food": condition_data.is_rotted_food,
                "total_food": condition_data.total_food,
                "last_open": condition_data.last_open,
                "is_saving_mode": condition_data.is_saving_mode
            }

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(response).encode('utf-8'))

        except Exception as e:
            print(f"Lỗi trong get_setting: {e}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to get setting")

    def do_GET(self):
        if self.path == "/receiveSetting":
            self.get_setting()
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b"Not found")

def signal_handler(sig, frame):
    print("\nĐang đóng server...")
    print("Server đã đóng")
    sys.exit(0)

def main():
    port = 8888
    server_address = ('', port)
    httpd = HTTPServer(server_address, Handler)
    signal.signal(signal.SIGINT, signal_handler)

    local_ip = Handler.get_local_ip() # Note: Sau này đổi thành IP server 
    print(f"Server đang chạy tại IP: {local_ip}, cổng: {port}")

    # Khởi chạy luồng gửi setting
    # TODO: Đang đợi để test tiếp
    # threading.Thread(target=send_setting_to_esp, daemon=True).start()
    # threading.Thread(target=send_test_data_to_esp, daemon=True).start()

    # Khởi chạy luồng theo dõi thay đổi thực phẩm
    threading.Thread(target=monitor_food_changes, daemon=True).start()

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        signal_handler(None, None)

if __name__ == "__main__":
    main()