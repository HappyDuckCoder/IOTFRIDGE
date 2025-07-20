import signal
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path
from util import *

sys.path.append(str(Path(__file__).resolve().parent.parent))

from thirdparty.api.SpeechToText import AudioModel
# from thirdparty.api.ClassificationFunction import *

modelSTT = AudioModel()
modelSTT.load()

class Handler(BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.file_name = "./resources/recording.wav"
        self.file_name_text = "./resources/text.txt"
        # Đảm bảo thư mục resources tồn tại
        Path("./resources").mkdir(exist_ok=True)
        super().__init__(*args, **kwargs)
    
    def post_audio(self): 
        try:
            # Lấy content length
            content_length = int(self.headers.get('Content-Length', 0))
            
            # Đọc dữ liệu audio
            audio_data = self.rfile.read(content_length)
            
            # Lưu file audio
            # lưu file tạm
            with open("temp.pcm", "wb") as f:
                f.write(audio_data)

            # chuyển sang WAV chuẩn
            convert_pcm_to_wav("temp.pcm", self.file_name)
            
            # Trả về kết quả
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain; charset=utf-8')
            self.end_headers()

            # Speech to Text
            text = modelSTT.transcribe(self.file_name)

            with open(self.file_name_text, 'wb') as f:
                f.write(text.encode('utf-8'))

            # inp = text

            # # Classification
            # # Chia thành phần
            # component = DevideComponentInInput(inp)

            # if component is None:
            #     print("Lỗi khi phân tích câu lệnh. Vui lòng thử lại.")
            #     return
            
            # # Phân loại hành động
            # classified_action = classifyAction(component.action)
            
            # if classified_action is None:
            #     print("Không thể phân loại hành động.")
            #     return
            
            # # Thực hiện task
            # result = executeTask(component, classified_action)
            # if result:
            #     print(f"Output: {result}")
            
        except Exception as error:
            print(f"Error transcribing audio: {error}")
            self.send_response(500)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"Error processing audio")

    def post_data(self):

        # nhận các data và gửi lên firebase (tham khảo model food)

        # Bước 1: tạo model Condition gồm các data ---> vào model.py
        # Bước 2: tạo method cho Condition (cụ thể là POST) ---> vào method.py

        pass # đoàn làm

    def do_POST(self):
        if self.path == "/uploadAudio":
            self.post_audio()
        elif self.path == "/uploadData":  
            self.post_data()
        else:
            print("Error: Check your POST request")
            self.send_response(405)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"Method not allowed")


def signal_handler(sig, frame):
    print("\nĐang đóng server...")
    print("Server đã đóng")
    sys.exit(0)

def main():
    port = 8888
    server_address = ('', port)
    
    # Tạo HTTP server
    httpd = HTTPServer(server_address, Handler)
    
    # Đăng ký signal handler cho graceful shutdown
    signal.signal(signal.SIGINT, signal_handler)
    
    print(f"Server đang chạy tại cổng {port}")
    print(f"Sử dụng POST request đến http://localhost:{port}/uploadAudio")
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        signal_handler(None, None)

if __name__ == "__main__":
    main()