# Class Food (thực phẩm) 
# Đại diện cho một món thực phẩm với các thuộc tính:
# id, name, quantity, unit, good (trạng thái tốt), expired (trạng thái hỏng)

class Food:
    def __init__(self, id, name, quantity, unit, good, expired):
        self.id = id                  # Mã định danh duy nhất
        self.name = name              # Tên thực phẩm (ví dụ: táo, sữa)
        self.quantity = quantity      # Số lượng
        self.unit = unit              # Đơn vị (kg, lít, hộp...)
        self.good = good              # Trạng thái còn tốt (ví dụ: "còn tươi")
        self.expired = expired        # Trạng thái đã hư (ví dụ: "bị hư")

    # Chuyển đối tượng thành dict (dễ lưu trữ hoặc chuyển JSON)
    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "quantity": self.quantity,
            "unit": self.unit,
            "good": self.good,
            "expired": self.expired
        }

    # Tạo đối tượng từ dict (dữ liệu từ file hoặc API)
    @staticmethod
    def from_dict(data):
        return Food(
            id=data.get("id"),
            name=data.get("name"),
            quantity=data.get("quantity"),
            unit=data.get("unit"),
            good=data.get("good", "còn tươi"),         # Nếu thiếu, mặc định là còn tươi
            expired=data.get("expired", "bị hư")       # Nếu thiếu, mặc định là bị hư
        )
    
# class Fridge: chứa danh sách Food, quản lý thêm/xóa/sửa
# class User: thông tin người dùng, có thể liên kết với một hoặc nhiều Fridge
