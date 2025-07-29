import signal
import sys
import json
import socket
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

class Handler(BaseHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        self.file_name = "./resources/recording.wav"
        self.file_name_text = "./resources/text.txt"
        Path("./resources").mkdir(exist_ok=True)
        super().__init__(*args, **kwargs)

    def get_local_ip():
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))  # IP bất kỳ, không cần truy cập được
            ip = s.getsockname()[0]
            s.close()
            return ip
        except Exception:
            return "127.0.0.1"

    # Nhận file âm thanh
    def post_audio(self): 
        try:
            content_length = int(self.headers['Content-Length'])
            audio_data = self.rfile.read(content_length)

            with open(self.file_name, 'wb') as f:
                f.write(audio_data)

            print("Đã nhận file ghi âm từ ESP32")
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b"Audio received successfully")

        except Exception as error:
            print(f"Lỗi trong post_audio: {error}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to receive audio")

    # Nhận dữ liệu test JSON
    def post_test_data(self):
        try:
            content_length = int(self.headers['Content-Length'])
            raw_data = self.rfile.read(content_length)
            json_data = json.loads(raw_data.decode('utf-8'))

            print("Dữ liệu test nhận từ ESP32:")
            print(json.dumps(json_data, indent=2))

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"status": "test received"}')

        except Exception as error:
            print(f"Lỗi trong post_test_data: {error}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to receive test data")

    # Nhận dữ liệu chính (ví dụ gửi lên Firebase sau này)
    def post_data(self):
        try:
            content_length = int(self.headers['Content-Length'])
            raw_data = self.rfile.read(content_length)
            json_data = json.loads(raw_data.decode('utf-8'))

            print("Dữ liệu thật nhận từ ESP32:")
            print(json.dumps(json_data, indent=2))

            # TODO: Gửi lên Firebase tại đây (sau)
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"status": "data received"}')

        except Exception as error:
            print(f"Lỗi trong post_data: {error}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Failed to receive data")

    def do_POST(self):
        if self.path == "/uploadAudio":
            self.post_audio()
        elif self.path == "/uploadData":
            self.post_data()
        elif self.path == "/uploadTestData":
            self.post_test_data()
        else:
            print("Error: Đường dẫn không hợp lệ")
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
    httpd = HTTPServer(server_address, Handler)
    signal.signal(signal.SIGINT, signal_handler)

    local_ip = Handler.get_local_ip() # Note: Sau này đổi thành IP server 
    print(f"Server đang chạy tại IP: {local_ip}, cổng: {port}")

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        signal_handler(None, None)

if __name__ == "__main__":
    main()
