# class Food 
# --> id, name, quantity, unit, image=null, state 

class Food:
    def __init__(self, id, name, quantity, unit, good, expired):
        self.id = id
        self.name = name
        self.quantity = quantity
        self.unit = unit
        self.good = good  # "good"
        self.expired = expired  # "expired"

    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "quantity": self.quantity,
            "unit": self.unit,
            "good": self.good,
            "expired": self.expired
        }

    @staticmethod
    def from_dict(data):
        return Food(
            id=data.get("id"),
            name=data.get("name"),
            quantity=data.get("quantity"),
            unit=data.get("unit"),
            good=data.get("good", "còn tươi"),
            expired=data.get("expired", "bị hư")
        )
    
# class Fridge
# class User