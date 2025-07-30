from my_util import convert_pcm_to_wav
from thirdparty.services.handleTask import TaskHandling
from thirdparty.api.SpeechToText import AudioModel


def convert_autio(inp_path, out_path):
    convert_pcm_to_wav(inp_path, out_path)

def main():
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

if __name__ == "__main__":
    main()