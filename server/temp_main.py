#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
temp_main.py - Tích hợp Speech-to-Text và Classification Function

Flow:
1. Input: giọng của người dùng: "thêm 1 quả táo"
2. STT (Speech-to-Text): "thêm 1 quả táo" 
3. Classification: thêm / 1 / quả / táo 
4. Thực hiện hàm thêm trong Classification (kết hợp với database)
"""

import os
import sys
import time

# Thêm thư mục hiện tại vào path
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_dir)

from SpeechToText import Recorder, AudioModel
from ClassificationFunction import process_command
from thirdparty.database.method import get_all_foods
from utils import format_food_display

class SmartFridgeVoiceInterface:
    def __init__(self):
        print("🎤 Khởi tạo Smart Fridge Voice Interface...")
        
        # Khởi tạo recorder
        self.recorder = Recorder(duration=5, fs=16000, save_path="resources/audio/voice_command.wav")
        
        # Khởi tạo speech-to-text model
        self.stt_model = AudioModel(model_name="tranviethuy01/whisper-medium-vi", max_length=100)
        print("📡 Đang tải mô hình Speech-to-Text...")
        self.stt_model.load()
        print("✅ Khởi tạo hoàn tất!")
    
    def listen_and_process(self):
        """Nghe lệnh giọng nói và xử lý"""
        try:
            # Ghi âm
            print("\n🎙️  Hãy nói lệnh của bạn...")
            self.recorder.record_to_file()
            
            # Chuyển đổi speech-to-text
            print("🔄 Đang nhận diện giọng nói...")
            text_command = self.stt_model.transcribe(self.recorder.save_path)
            
            print(f"📝 Lệnh được nhận diện: '{text_command}'")
            
            if not text_command.strip():
                print("❌ Không nhận diện được lệnh. Vui lòng thử lại.")
                return
            
            # Xử lý lệnh bằng Classification Function
            print("⚙️  Đang xử lý lệnh...")
            result = process_command(text_command)
            
            print(f"✅ Kết quả: {result}")
            
        except Exception as e:
            print(f"❌ Lỗi: {e}")
    
    def show_current_foods(self):
        """Hiển thị danh sách thực phẩm hiện có"""
        print("\n📦 Danh sách thực phẩm trong tủ lạnh:")
        print("-" * 50)
        
        foods = get_all_foods()
        if not foods:
            print("Tủ lạnh đang trống.")
        else:
            for i, food in enumerate(foods, 1):
                print(f"{i}. {format_food_display(food)}")
        
        print("-" * 50)
    
    def run(self):
        """Chạy giao diện chính"""
        print("\n🏠 Chào mừng đến với Smart Fridge Voice Interface!")
        print("Bạn có thể:")
        print("- Nói lệnh để thêm/xóa/sửa thực phẩm")
        print("- Xem danh sách thực phẩm hiện có")
        
        while True:
            print("\n" + "="*60)
            print("🔧 Chọn chức năng:")
            print("1. 🎤 Nói lệnh (Voice Command)")
            print("2. ⌨️  Nhập lệnh bằng tay (Text Command)")
            print("3. 📋 Xem danh sách thực phẩm")
            print("4. 🚪 Thoát")
            
            choice = input("\nNhập lựa chọ