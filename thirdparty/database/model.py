class Food:
    def __init__(self, id, name, quantity, unit, is_good, is_expired, input_date, output_date, image_url=None):
        self.id = id
        self.name = name
        self.quantity = quantity
        self.unit = unit
        self.is_good = is_good
        self.is_expired = is_expired
        self.input_date = input_date
        self.output_date = output_date
        self.image_url = image_url

    def to_dict(self):
        return {
            "id": self.id,
            "name": self.name,
            "quantity": self.quantity,
            "unit": self.unit,
            "is_good": self.is_good,
            "is_expired": self.is_expired,
            "input_date": self.input_date,
            "output_date": self.output_date,
            "image_url": self.image_url
        }

    @staticmethod
    def from_dict(data):
        return Food(
            data.get("id"),
            data.get("name"),
            data.get("quantity"),
            data.get("unit"),
            data.get("is_good", True),
            data.get("is_expired", False),
            data.get("input_date"),
            data.get("output_date"),
            data.get("image_url")
        )

    def __str__(self):
        return f"{self.name} ({self.quantity} {self.unit})"