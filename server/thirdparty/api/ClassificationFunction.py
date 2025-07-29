import google.generativeai as genai
import json

API_GEMINI_KEY = "AIzaSyB5ZEm_hOqAf7APH3dzVSQ7_2Ezn_IYVn8"
model_gemini_name = "gemini-1.5-flash" 
genai.configure(api_key=API_GEMINI_KEY)

class Model: 
    def __init__(self, model_name):
        self.model_name = model_name
        self.model = None
    
    def runModel(self):
        self.model = genai.GenerativeModel(
            model_name=self.model_name
        )
    
    def generate_content(self, prompt):
        if self.model is None:
            raise ValueError("Model chưa được khởi tạo. Hãy gọi runModel() trước.")
        return self.model.generate_content(prompt)

class Task:
    def __init__(self, action, quantity, unit, food, old_food=None, old_quantity=None, old_unit=None):
        self.action = action
        self.quantity = quantity
        self.unit = unit
        self.food = food
        # Thêm các trường cho chức năng sửa
        self.old_food = old_food
        self.old_quantity = old_quantity
        self.old_unit = old_unit
    
    def __repr__(self):
        if self.old_food or self.old_quantity or self.old_unit:
            return f"{self.action}: {self.old_food or self.food} {self.old_quantity or self.quantity} {self.old_unit or self.unit} -> {self.food} {self.quantity} {self.unit}"
        return f"{self.action}: {self.quantity} {self.unit} {self.food}"

def DevideComponentInInput(text: str):
    model = Model(model_gemini_name)
    model.runModel()
    
    prompt = f"""
Hãy phân tích câu tiếng Việt sau và xuất ra kết quả dưới dạng JSON.

Câu: "{text}"

Nếu là hành động THÊM hoặc XÓA, trả về JSON:
{{
  "action": "...",
  "quantity": "...",
  "unit": "...",
  "food": "..."
}}

Nếu là hành động SỬA/THAY ĐỔI (ví dụ: "sửa 2 kg gạo thành 3 kg gạo", "thay đổi táo thành cam"), trả về JSON:
{{
  "action": "sửa",
  "quantity": "số lượng mới",
  "unit": "đơn vị mới", 
  "food": "thực phẩm mới",
  "old_quantity": "số lượng cũ",
  "old_unit": "đơn vị cũ",
  "old_food": "thực phẩm cũ"
}}

Chú ý: 
- Với hành động sửa, cần xác định rõ thông tin cũ và mới
- Nếu chỉ sửa một phần (ví dụ chỉ số lượng), thì các trường khác giữ nguyên
- Hành động có thể là: thêm, xóa, sửa, thay đổi, chỉnh sửa, cập nhật
""".strip()
    
    try:
        response = model.generate_content(prompt)
        response_text = response.text.strip()
        
        # Tìm và extract JSON từ response
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
        
        # Lấy thông tin cơ bản
        action = json_data.get("action", "")
        quantity = json_data.get("quantity", "")
        unit = json_data.get("unit", "")
        food = json_data.get("food", "")
        
        # Lấy thông tin cũ nếu có (cho chức năng sửa)
        old_food = json_data.get("old_food", None)
        old_quantity = json_data.get("old_quantity", None)
        old_unit = json_data.get("old_unit", None)
        
        return Task(action, quantity, unit, food, old_food, old_quantity, old_unit)
        
    except json.JSONDecodeError as e:
        print(f"Lỗi JSON decode: {e}")
        if 'response' in locals():
            print(f"Response text: {response.text}")
        return None
    except Exception as e:
        print(f"Lỗi khi phân tích: {e}")
        return None

def classifyAction(action: str):
    model = Model(model_gemini_name)
    model.runModel()
    
    prompt = f"""
Hãy phân loại hành động "{action}" thành một trong ba loại: "thêm", "xóa", hoặc "sửa".
Chỉ trả lời duy nhất một từ: "thêm", "xóa", hoặc "sửa".

Quy tắc phân loại:
- "thêm": thêm, bổ sung, đưa vào, cho thêm
- "xóa": xóa, loại bỏ, gỡ, bỏ, xóa bỏ
- "sửa": sửa, thay đổi, chỉnh sửa, cập nhật, thay thế, đổi
""".strip() 
    
    try:
        response = model.generate_content(prompt)
        response_text = response.text.strip().lower()
        
        # Đảm bảo kết quả chỉ là một trong ba từ
        if response_text in ["thêm", "xóa", "sửa"]:
            return response_text
        else:
            print(f"Hành động không được nhận diện: {response_text}")
            return None
            
    except Exception as e:
        print(f"Lỗi khi phân loại hành động: {e}")
        return None

# Note: sửa lại các Task with action theo food mới 

def TaskWithActionAdd(action: str, quantity: str, unit: str, food: str): 
    # In ra hành động
    print(f"THÊM: {quantity} {unit} {food}")
    
    add_food(Food(id=food, name=food, quantity=quantity, unit=unit, status="còn tồi"))

    return f"Đã thêm {quantity} {unit} {food}"

def TaskWithActionDelete(action: str, quantity: str, unit: str, food: str):
    print(f"XÓA: {quantity} {unit} {food}")
    
    eat_food(Food(id=food, name=food, quantity=quantity, unit=unit, status="còn tồi"))

    return f"Đã xóa {quantity} {unit} {food}"

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

    # update_food(Food(id=task.food, name=task.food, quantity=task.quantity, unit=task.unit, status="còn tồi"))
    
    return f"Đã sửa {task.old_quantity} {task.old_unit} {task.old_food} thành {task.quantity} {task.unit} {task.food}"

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

def main():
    # Nhận đầu vào từ người dùng
    inp = input("Nhập lệnh: ").strip()
    
    # Chia thành phần
    component = DevideComponentInInput(inp)

    if component is None:
        print("Lỗi khi phân tích câu lệnh. Vui lòng thử lại.")
        return
    
    # Phân loại hành động
    classified_action = classifyAction(component.action)
    
    if classified_action is None:
        print("Không thể phân loại hành động.")
        return
    
    # Thực hiện task
    result = executeTask(component, classified_action)
    if result:
        print(f"Output: {result}")

if __name__ == "__main__":
    main()