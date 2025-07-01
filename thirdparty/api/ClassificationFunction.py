# Nhập thư viện Gemini AI và JSON
import google.generativeai as genai
import json

# Nhập các hàm thao tác database từ module riêng
from thirdparty.database.method import *

# Cấu hình API key và tên mô hình Gemini
API_GEMINI_KEY = "AIzaSyB5ZEm_hOqAf7APH3dzVSQ7_2Ezn_IYVn8"
model_gemini_name = "gemini-1.5-flash" 

genai.configure(api_key=API_GEMINI_KEY)

# Lớp dùng để tương tác với mô hình Gemini
class Model: 
    def __init__(self, model_name):
        self.model_name = model_name
        self.model = None
    
    # Khởi tạo mô hình
    def runModel(self):
        self.model = genai.GenerativeModel(
            model_name=self.model_name
        )
    
    # Gửi prompt và nhận phản hồi
    def generate_content(self, prompt):
        if self.model is None:
            raise ValueError("Model chưa được khởi tạo. Hãy gọi runModel() trước.")
        return self.model.generate_content(prompt)

# Lớp biểu diễn một tác vụ (task) như thêm, xóa, sửa thực phẩm
class Task:
    def __init__(self, action, quantity, unit, food, old_food=None, old_quantity=None, old_unit=None):
        self.action = action
        self.quantity = quantity
        self.unit = unit
        self.food = food
        self.old_food = old_food
        self.old_quantity = old_quantity
        self.old_unit = old_unit
    
    def __repr__(self):
        # Hiển thị task ở dạng dễ hiểu, hỗ trợ cả hành động sửa
        if self.old_food or self.old_quantity or self.old_unit:
            return f"{self.action}: {self.old_food or self.food} {self.old_quantity or self.quantity} {self.old_unit or self.unit} -> {self.food} {self.quantity} {self.unit}"
        return f"{self.action}: {self.quantity} {self.unit} {self.food}"

# Phân tích đầu vào tiếng Việt thành JSON mô tả hành động
def DevideComponentInInput(text: str):
    model = Model(model_gemini_name)
    model.runModel()
    
    # Prompt yêu cầu Gemini phân tích câu nhập
    prompt = f"""...""".strip()  # (giữ nguyên phần prompt dài, đã rõ ràng)

    try:
        response = model.generate_content(prompt)
        response_text = response.text.strip()
        
        # Trích xuất phần JSON từ phản hồi
        start_idx = response_text.find('{')
        end_idx = response_text.rfind('}') + 1
        
        if start_idx != -1 and end_idx > start_idx:
            json_str = response_text[start_idx:end_idx]
            json_data = json.loads(json_str)
        else:
            raise ValueError("Không tìm thấy JSON trong response")
        
        # Kiểm tra các trường bắt buộc
        required_fields = ["action", "quantity", "unit", "food"]
        missing_fields = []
        
        for field in required_fields:
            if field not in json_data or not json_data[field] or json_data[field].strip() == "":
                missing_fields.append(field)
        
        if missing_fields:
            print(f"Lỗi: Thiếu trường bắt buộc: {', '.join(missing_fields)}")
            return None
        
        # Tạo đối tượng Task từ dữ liệu JSON
        return Task(
            json_data.get("action", ""),
            json_data.get("quantity", ""),
            json_data.get("unit", ""),
            json_data.get("food", ""),
            json_data.get("old_food", None),
            json_data.get("old_quantity", None),
            json_data.get("old_unit", None)
        )
        
    except json.JSONDecodeError as e:
        print(f"Lỗi JSON decode: {e}")
        if 'response' in locals():
            print(f"Response text: {response.text}")
        return None
    except Exception as e:
        print(f"Lỗi khi phân tích: {e}")
        return None

# Phân loại hành động (thêm/xóa/sửa) từ văn bản
def classifyAction(action: str):
    model = Model(model_gemini_name)
    model.runModel()
    
    # Prompt yêu cầu Gemini phân loại hành động
    prompt = f"""...""".strip()  # (giữ nguyên prompt)

    try:
        response = model.generate_content(prompt)
        response_text = response.text.strip().lower()
        
        if response_text in ["thêm", "xóa", "sửa"]:
            return response_text
        else:
            print(f"Hành động không được nhận diện: {response_text}")
            return None
            
    except Exception as e:
        print(f"Lỗi khi phân loại hành động: {e}")
        return None

# Thêm thực phẩm
def TaskWithActionAdd(action: str, quantity: str, unit: str, food: str): 
    print(f"THÊM: {quantity} {unit} {food}")
    add_food(Food(id=food, name=food, quantity=quantity, unit=unit, status="còn tồi"))
    return f"Đã thêm {quantity} {unit} {food}"

# Xóa thực phẩm
def TaskWithActionDelete(action: str, quantity: str, unit: str, food: str):
    print(f"XÓA: {quantity} {unit} {food}")
    delete_food(Food(id=food, name=food, quantity=quantity, unit=unit, status="còn tồi"))
    return f"Đã xóa {quantity} {unit} {food}"

# Sửa thực phẩm
def TaskWithActionEdit(task: Task):
    print(f"SỬA: ", end="")
    
    changes = []
    if task.old_food and task.old_food != task.food:
        changes.append(f"thực phẩm từ '{task.old_food}' thành '{task.food}'")
    if task.old_quantity and task.old_quantity != task.quantity:
        changes.append(f"số lượng từ '{task.old_quantity}' thành '{task.quantity}'")
    if task.old_unit and task.old_unit != task.unit:
        changes.append(f"đơn vị từ '{task.old_unit}' thành '{task.unit}'")
    
    if changes:
        print("Thay đổi " + ", ".join(changes))
    else:
        print(f"Cập nhật thành {task.quantity} {task.unit} {task.food}")

    # update_food(...) bị tạm thời comment
    return f"Đã sửa {task.old_quantity} {task.old_unit} {task.old_food} thành {task.quantity} {task.unit} {task.food}"

# Thực thi task dựa vào loại hành động đã qua filter
def executeTask(task: Task, classified_action: str):
    if classified_action == "thêm":
        return TaskWithActionAdd(classified_action, task.quantity, task.unit, task.food)
    elif classified_action == "xóa":
        return TaskWithActionDelete(classified_action, task.quantity, task.unit, task.food)
    elif classified_action == "sửa":
        return TaskWithActionEdit(task)
    else:
        print(f"Không thể thực hiện hành động: {classified_action}")
        return None

# Main function để chạy chương trình
def main():
    inp = input("Nhập lệnh: ").strip()  # Nhận input từ người dùng
    
    component = DevideComponentInInput(inp)  # Phân tích câu thành task

    if component is None:
        print("Lỗi khi phân tích câu lệnh. Vui lòng thử lại.")
        return
    
    classified_action = classifyAction(component.action)  # Phân loại hành động
    
    if classified_action is None:
        print("Không thể phân loại hành động.")
        return
    
    result = executeTask(component, classified_action)  # Thực hiện hành động
    if result:
        print(f"Output: {result}")

if __name__ == "__main__":
    main()
