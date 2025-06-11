# class Food 
# --> id, name, quantity, unit, image=null, state 

class Food:
    def __init__(self, id, name, quantity, unit, status):
        self.id = id
        self.name = name
        self.quantity = quantity
        self.unit = unit
        self.status = status  # "good" hoặc "expired"

    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "quantity": self.quantity,
            "unit": self.unit,
            "status": self.status
        }

    @staticmethod
    def from_dict(data):
        return Food(
            id=data.get("id"),
            name=data.get("name"),
            quantity=data.get("quantity"),
            unit=data.get("unit"),
            status=data.get("status")
        )