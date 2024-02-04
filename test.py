from pymongo import MongoClient
import uart
from queue import Queue
import threading
from license_plate import license_detect, license_to_string
import re
import db_com
import datetime
import whisper
from openai import OpenAI
import speech_to_gpt
from time import sleep
import sounddevice as sd

# 음성 AI를 위한 초기화
model = whisper.load_model('base')
client = OpenAI(api_key='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')
sd.default.device(kind='input', device=4)
speech_to_gpt_stop = False
voice_interface_thread = None

# def input_thread_function():
#     global speech_to_gpt_stop, record_count, model, client
#     while True:
#         user_input = input("음성대화를 시작하고 싶으면 1을, 종료하고 싶으면 2를 눌러주세요: ")
#         if user_input == '1':
#             speech_to_gpt.speech_to_gpt(model, client)
#         elif user_input == '2':
#             break

# input_thread = threading.Thread(target=input_thread_function)
# input_thread.start()

a = 0
while True:
    print(a)
    sleep(1)
    a += 1