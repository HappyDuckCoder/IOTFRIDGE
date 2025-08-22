from thirdparty.api.NLPapi import Model, model_gemini_name
import json
from my_util import get_default_expired_date, get_category, get_image_url, generate_random_id_string, string_to_number
from thirdparty.database.method import add_food, eat_food
from thirdparty.database.model import Food  
from datetime import datetime
from thirdparty.api.GetCalo import GetCaloService

def get_calo_usda(food_name, quantity, unit): 
    service = GetCaloService()
    return service.get_calo_usda(food_name) # *NOTE: CODE DƠ 

class Task:
    def __init__(self, action, quantity, unit, food, old_food=None, old_quantity=None, old_unit=None):
        self.action = action
        self.quantity = quantity
        self.unit = unit
        self.food = food
        # Thêm các trường cho chức năng sửa, tạm thời đang bỏ chức năng sửa
        self.old_food = old_food
        self.old_quantity = old_quantity
        self.old_unit = old_unit
    
    def __repr__(self):
        if self.old_food or self.old_quantity or self.old_unit:
            return f"{self.action}: {self.old_food or self.food} {self.old_quantity or self.quantity} {self.old_unit or self.unit} -> {self.food} {self.quantity} {self.unit}"
        return f"{self.action}: {self.quantity} {self.unit} {self.food}"

class TaskHandling():
    def __init__(self):
        pass

    def DevideComponentInInput(self, text: str):
        if text is None or text.strip() == "":
            raise ValueError("Vui lòng nhập vào một chuỗi không rỗng")
        
        model = Model(model_gemini_name)
        if model is None:
            raise ValueError("Model gemini_name is None")
        model.runModel()
        
        prompt = f"""
    Hãy phân tích câu tiếng Việt sau và xuất ra kết quả dướidạng JSON.
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

    def classifyAction(self, action: str):
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

    def TaskWithActionAdd(self, action: str, quantity: str, unit: str, food: str): 
        # In ra hành động
        print(f"{action}: {quantity} {unit} {food}")
        
        food = Food(
            id=generate_random_id_string(),  # tạo id ngẫu nhiên
            name=food, 
            quantity=string_to_number(quantity), 
            unit=unit, 
            is_good=True, 
            is_expired=False, 
            input_date=datetime.now(), # ngày hiện tại
            output_date=get_default_expired_date(7), # tạm thời
            category=get_category(food), # tạo 1 trong 4 default category
            calo=get_calo_usda(food, quantity, unit), # api calo
            image_url=get_image_url(food) # api image
        )

        add_food(food)

        return f"Đã thêm {quantity} {unit} {food}"

    def TaskWithActionDelete(self, action: str, quantity: str, unit: str, food: str):
        print(f"{action}: {quantity} {unit} {food}")
        
        eat_food(food, quantity)

        return f"Đã xóa {quantity} {unit} {food}"

    def executeTask(self, task: Task, classified_action: str):
        if classified_action == "thêm":
            return self.TaskWithActionAdd(classified_action, task.quantity, task.unit, task.food)
        elif classified_action == "xóa":
            return self.TaskWithActionDelete(classified_action, task.quantity, task.unit, task.food)
        else:
            return f"Không thể thực hiện hành động: {classified_action}"
    
def main():
    taskHandling = TaskHandling()

    # Nhận đầu vào từ người dùng
    inp = input("Nhập lệnh: ").strip()
    
    # Chia thành phần
    component = taskHandling.DevideComponentInInput(inp)

    if component is None:
        print("Lỗi khi phân tích câu lệnh. Vui lòng thử lại.")
        return
    
    # Phân loại hành động
    classified_action = taskHandling.classifyAction(component.action)
    
    if classified_action is None:
        print("Không thể phân loại hành động.")
        return
    
    # Thực hiện task
    result = taskHandling.executeTask(component, classified_action)
    if result:
        print(f"Output: {result}")

if __name__ == "__main__":
    main()