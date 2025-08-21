from datetime import datetime, date
from my_util import convert_to_datetime


class Food:
    def __init__(
        self, id: str, name: str, quantity: float, unit: str,
        is_good: bool, is_expired: bool, input_date: datetime, output_date: datetime,
        category: str, calo: float, user_preference: int = 6,
        image_url=None
    ):
        self.id = id
        self.name = name
        self.quantity = quantity
        self.unit = unit
        self.is_good = is_good
        self.is_expired = is_expired
        self.input_date = input_date
        self.output_date = output_date
        self.category = category
        self.calo = calo
        self.image_url = image_url

        # for food suggestion
        self.user_preference = user_preference

    def to_dict(self):
        def ensure_datetime(value):
            if isinstance(value, date) and not isinstance(value, datetime):
                return datetime.combine(value, datetime.min.time())
            return value

        return {
            "name": self.name,
            "quantity": self.quantity,
            "unit": self.unit,
            "is_good": self.is_good,
            "is_expired": self.is_expired,
            "input_date": ensure_datetime(self.input_date),
            "output_date": ensure_datetime(self.output_date),
            "category": self.category,
            "calo": self.calo,
            "user_preference": self.user_preference,
            "image_url": self.image_url,
        }

    @staticmethod
    def from_dict(id, data):
        return Food(
            id=id,
            name=data.get("name"),
            quantity=data.get("quantity"),
            unit=data.get("unit"),
            is_good=data.get("is_good"),
            is_expired=data.get("is_expired"),
            input_date=convert_to_datetime(data.get("input_date")),
            output_date=convert_to_datetime(data.get("output_date")),
            category=data.get("category"),
            calo=data.get("calo"),
            user_preference=data.get("user_preference"),
            image_url=data.get("image_url"),
        )

class User:
    def __init__(self, id: str, phone: str, password: str, detail_user_id: str, 
                 fridge_data_ids = None):
        self.id = id
        self.phone = phone
        self.password = password
        self.detail_user_id = detail_user_id
        self.fridge_data_ids = fridge_data_ids if fridge_data_ids else []

    def to_dict(self):
        return {
            "phone": self.phone,
            "password": self.password,
            "detail_user_id": self.detail_user_id,
            "fridge_data_ids": self.fridge_data_ids
        }

    @staticmethod
    def from_dict(id: str, data):
        return User(
            id=id,
            phone=data.get("phone"),
            password=data.get("password"),
            detail_user_id=data.get("detail_user_id"),
            fridge_data_ids=data.get("fridge_data_ids", [])
        )


class UserDetail:
    def __init__(self, id: str, name: str, age: int, dob: datetime, 
                 email: str, gender: str, address: str):
        self.id = id
        self.name = name
        self.age = age
        self.dob = dob
        self.email = email
        self.gender = gender
        self.address = address

    def to_dict(self):
        return {
            "name": self.name,
            "age": self.age,
            "dob": self.dob,
            "email": self.email,
            "gender": self.gender,
            "address": self.address
        }

    @staticmethod
    def from_dict(id: str, data):
        return UserDetail(
            id=id,
            name=data.get("name"),
            age=data.get("age"),
            dob=data.get("dob"),
            email=data.get("email"),
            gender=data.get("gender"),
            address=data.get("address")
        )


class FridgeData:
    def __init__(self, id: str, condition_id: str, setting_id: str, wifi_data_id: str,
                 food_ids = None, user_ids = None):
        self.id = id
        self.condition_id = condition_id
        self.setting_id = setting_id
        self.wifi_data_id = wifi_data_id
        self.food_ids = food_ids if food_ids else []
        self.user_ids = user_ids if user_ids else []

    def to_dict(self):
        return {
            "condition_id": self.condition_id,
            "setting_id": self.setting_id,
            "wifi_data_id": self.wifi_data_id,
            "food_ids": self.food_ids,
            "user_ids": self.user_ids
        }

    @staticmethod
    def from_dict(id: str, data):
        return FridgeData(
            id=id,
            condition_id=data.get("condition_id"),
            setting_id=data.get("setting_id"),
            wifi_data_id=data.get("wifi_data_id"),
            food_ids=data.get("food_ids", []),
            user_ids=data.get("user_ids", [])
        )


class WifiData:
    def __init__(self, id: str, user_id: str, ssid: str, wifi_password: str, is_connect: bool):
        self.id = id
        self.user_id = user_id
        self.ssid = ssid
        self.wifi_password = wifi_password
        self.is_connect = is_connect

    def to_dict(self):
        return {
            "user_id": self.user_id,
            "ssid": self.ssid,
            "wifi_password": self.wifi_password,
            "is_connect": self.is_connect
        }

    @staticmethod
    def from_dict(id: str, data):
        return WifiData(
            id=id,
            user_id=data.get("user_id"),
            ssid=data.get("ssid"),
            wifi_password=data.get("wifi_password"),
            is_connect=data.get("is_connect", False)
        )


class Setting:
    def __init__(self, id: str, fan_mode: int, diet: str, is_open_notification: bool, 
                 fridge_data_id: str):
        self.id = id
        self.fan_mode = fan_mode
        self.diet = diet  
        self.is_open_notification = is_open_notification
        self.fridge_data_id = fridge_data_id

    def to_dict(self):
        return {
            "fan_mode": self.fan_mode,
            "diet": self.diet,
            "is_open_notification": self.is_open_notification,
            "fridge_data_id": self.fridge_data_id
        }

    @staticmethod
    def from_dict(id: str, data):
        return Setting(
            id=id,
            fan_mode=data.get("fan_mode"),
            diet=data.get("diet"),
            is_open_notification=data.get("is_open_notification", False),
            fridge_data_id=data.get("fridge_data_id")
        )


class Condition:
    def __init__(self, id: str, last_open: int, humi: float, 
                 temp: float, is_rotted_food: bool, total_food: int, is_saving_mode: bool,
                 fridge_data_id: str):
        self.id = id
        self.temp = temp
        self.humi = humi
        self.is_rotted_food = is_rotted_food
        self.total_food = total_food
        self.last_open = last_open
        self.is_saving_mode = is_saving_mode
        self.fridge_data_id = fridge_data_id

    def to_dict(self):
        return {
            # "id": self.id,
            "temp": self.temp,
            "humi": self.humi,
            "is_rotted_food": self.is_rotted_food,
            "total_food": self.total_food,
            "last_open": self.last_open,
            "is_saving_mode": self.is_saving_mode,
            "fridge_data_id": self.fridge_data_id
        }

    @staticmethod
    def from_dict(id: str, data):
        return Condition(
            id=id,
            temp=data.get("temp"),
            humi=data.get("humi"),
            is_rotted_food=data.get("is_rotted_food"),
            total_food=data.get("total_food"),
            last_open=data.get("last_open"),
            is_saving_mode=data.get("is_saving_mode"),
            fridge_data_id=data.get("fridge_data_id")
        )

class Recipe:
    def __init__(self, id: str, title: str, cook_time: int, people_serving: int,
                 difficulty: str, instructions: str, ingredient_ids = None,
                image_url = None):
        self.id = id
        self.title = title
        self.cook_time = cook_time  # minutes
        self.people_serving = people_serving
        self.difficulty = difficulty  # "easy", "medium", "hard"
        self.instructions = instructions
        self.ingredient_ids = ingredient_ids if ingredient_ids else []
        self.image_url = image_url

    def to_dict(self):
        return {
            "title": self.title,
            "cook_time": self.cook_time,
            "people_serving": self.people_serving,
            "difficulty": self.difficulty,
            "instructions": self.instructions,
            "ingredient_ids": self.ingredient_ids,
            "image_url": self.image_url
        }

    @staticmethod
    def from_dict(id: str, data):
        return Recipe(
            id=id,
            title=data.get("title"),
            cook_time=data.get("cook_time"),
            people_serving=data.get("people_serving"),
            difficulty=data.get("difficulty"),
            instructions=data.get("instructions"),
            ingredient_ids=data.get("ingredient_ids", []),
            image_url=data.get("image_url")
        )


