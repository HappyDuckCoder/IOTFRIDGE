from my_util import convert_pcm_to_wav, translate_vietnamese_to_english
from thirdparty.services.handleTask import TaskHandling
from thirdparty.services.FoodSuggested import FoodSuggestedService
from thirdparty.api.SpeechToText import AudioModel
from thirdparty.database.method import get_all_foods
from thirdparty.api.GetRecipe import GetRecipeService
from thirdparty.database.method import add_recipe, get_setting_by_id

TOP_FOOD = 3
TOP_RECIPE = 5

def convert_audio(inp_path, out_path):
    convert_pcm_to_wav(inp_path, out_path)

def testMicFlow():
    stt = AudioModel()
    stt.load()

    text = stt.transcribe("resources/mic.wav")
    print("\nKết quả nhận diện:")
    print(text)

    taskHandling = TaskHandling()
    
    # Chia thành phần
    component = taskHandling.DevideComponentInInput(text)

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

def testCreateRecipeFlow():
    model_choose_ingredients = FoodSuggestedService()
    model_get_recipe = GetRecipeService()
    
    # lấy danh sách các môn ăn
    foods = get_all_foods()

    # lấy setting diet
    setting = get_setting_by_id("1")
    if setting:
        diet = setting.diet
    else:
        diet = None 

    top_suggestions = model_choose_ingredients.get_top_food_suggestions(foods, diet, top_n=TOP_FOOD)

    print(f"Top {TOP_FOOD} gợi ý:")
    for i, (food, score) in enumerate(top_suggestions, 1):
        print(f"  {i}. {food.name} - Điểm: {score:.3f} ({food.category})")
    print() 

    ingredients = [translate_vietnamese_to_english(food.name) for food, _ in top_suggestions]

    # ingredients_test = ['salmon', 'eggs']

    recipes = model_get_recipe.get_list_recipe_by_spoonacular(
        ingredients=ingredients,
        diet=diet,
        number=TOP_RECIPE
    )

    model_get_recipe.print_debug(recipes)

    for recipe in recipes:
        add_recipe(recipe)

def main():
    testCreateRecipeFlow()

if __name__ == "__main__":
    main()