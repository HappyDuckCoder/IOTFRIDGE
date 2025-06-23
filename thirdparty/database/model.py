# class Food 
# --> id, name, quantity, unit, image=null, state 

class Food:
    def __init__(self, id, name, quantity, unit):
        self.id = id
        self.name = name
        self.quantity = quantity
        self.unit = unit

    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "quantity": self.quantity,
            "unit": self.unit,
        }

    @staticmethod
    def from_dict(data):
        return Food(
            id=data.get("id"),
            name=data.get("name"),
            quantity=data.get("quantity"),
            unit=data.get("unit"),
        )
    
# class Fridge
# class User