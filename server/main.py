import signal
import sys
import os
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path
from util import *
sys.path.append(str(Path(__file__).resolve().parent.parent))
from thirdparty.api.SpeechToText import AudioModel
from thirdparty.api.ClassificationFunction import *

# Initialize model globally to avoid reloading for each request
modelSTT = AudioModel()
modelSTT.load()

class AudioHandler(BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.file_name = "./resources/recording.wav"
        self.file_name_text = "./resources/text.txt"
        # Đảm bảo thư mục resources tồn tại
        Path("./resources").mkdir(exist_ok=True)
        super().__init__(*args, **kwargs)
    
    def do_POST(self):
        if self.path == "/uploadAudio":
            try:
                # Lấy content length
                content_length = int(self.headers.get('Content-Length', 0))
                
                if content_length == 0:
                    self.send_response(400)
                    self.send_header('Content-Type', 'text/plain')
                    self.end_headers()
                    self.wfile.write(b"No audio data received")
                    return
                
                # Đọc dữ liệu audio
                audio_data = self.rfile.read(content_length)
                
                # Lưu file audio
                temp_file = "temp.pcm"
                try:
                    # Lưu file tạm
                    with open(temp_file, "wb") as f:
                        f.write(audio_data)
                    
                    # Chuyển sang WAV chuẩn
                    convert_pcm_to_wav(temp_file, self.file_name)
                    
                    # Speech to Text
                    text = modelSTT.transcribe(self.file_name)
                    
                    # Lưu text result
                    with open(self.file_name_text, 'w', encoding='utf-8') as f:
                        f.write(text)
                    
                    # Speech to Text
                    text = modelSTT.transcribe(self.file_name)
                    
                    # Lưu text result
                    with open(self.file_name_text, 'w', encoding='utf-8') as f:
                        f.write(text)
                    
                    print(f"Transcribed text: {text}")
                    
                    # Classification and Task Execution
                    inp = text
                    task_result = None
                    
                    try:
                        # Chia thành phần
                        component = DevideComponentInInput(inp)
                        if component is None:
                            print("Lỗi khi phân tích câu lệnh. Vui lòng thử lại.")
                            task_result = "Lỗi khi phân tích câu lệnh"
                        else:
                            # Phân loại hành động
                            classified_action = classifyAction(component.action)
                            
                            if classified_action is None:
                                print("Không thể phân loại hành động.")
                                task_result = "Không thể phân loại hành động"
                            else:
                                # Thực hiện task
                                result = executeTask(component, classified_action)
                                if result:
                                    print(f"Output: {result}")
                                    task_result = result
                                else:
                                    task_result = "Không có kết quả từ task"
                    
                    except Exception as classification_error:
                        print(f"Error in classification/execution: {classification_error}")
                        task_result = f"Lỗi xử lý: {str(classification_error)}"
                    
                    # Tạo response JSON với cả text và task result
                    response_data = {
                        "transcribed_text": text,
                        "task_result": task_result
                    }
                    
                    # Trả về kết quả
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json; charset=utf-8')
                    self.send_header('Access-Control-Allow-Origin', '*')  # For CORS if needed
                    self.end_headers()
                    
                    import json
                    self.wfile.write(json.dumps(response_data, ensure_ascii=False).encode('utf-8'))
                    
                finally:
                    # Cleanup temporary file
                    if os.path.exists(temp_file):
                        try:
                            os.remove(temp_file)
                        except OSError as e:
                            print(f"Warning: Could not remove temp file {temp_file}: {e}")
                
            except Exception as error:
                print(f"Error transcribing audio: {error}")
                self.send_response(500)
                self.send_header('Content-Type', 'text/plain')
                self.end_headers()
                self.wfile.write(f"Error processing audio: {str(error)}".encode('utf-8'))
        else:
            print(f"Error: Invalid endpoint {self.path}")
            self.send_response(404)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"Endpoint not found")
    
    def do_OPTIONS(self):
        # Handle CORS preflight requests
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

def signal_handler(sig, frame):
    print("\nĐang đóng server...")
    print("Server đã đóng")
    sys.exit(0)

def main():
    port = 8888
    server_address = ('', port)
    
    try:
        # Tạo HTTP server
        httpd = HTTPServer(server_address, AudioHandler)
        
        # Đăng ký signal handler cho graceful shutdown
        signal.signal(signal.SIGINT, signal_handler)
        
        print(f"Server đang chạy tại cổng {port}")
        print(f"Sử dụng POST request đến http://localhost:{port}/uploadAudio")
        print("Nhấn Ctrl+C để dừng server")
        
        httpd.serve_forever()
        
    except OSError as e:
        if e.errno == 48:  # Address already in use
            print(f"Lỗi: Cổng {port} đã được sử dụng. Vui lòng thử cổng khác hoặc dừng process đang sử dụng cổng này.")
        else:
            print(f"Lỗi khởi tạo server: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        signal_handler(None, None)
    except Exception as e:
        print(f"Lỗi không mong muốn: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()