import google.generativeai as genai

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

