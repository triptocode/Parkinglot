import socket
import sounddevice as sd
import wavio as wv
import whisper
import pyttsx3
from openai import OpenAI

"""
you need
sudo apt-get install espeak
&
pip install git+https://github.com/openai/whisper.git
"""

def start_voice_interface(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        server_socket.bind((host, port))
        server_socket.listen()
        print(f"voice interface waiting from {host}:{port}.")

        connection, address = server_socket.accept()
        with connection:
            print(f"connected to {address}")
            while True:
                data = connection.recv(4)
                if not data:
                    break
                print(f"receive order from rapi")
                return 


def speak(answer:str):
    engine = pyttsx3.init()
    voices = engine.getProperty('voices')
    engine.setProperty('voice', voices[0].id)
    engine.say(answer)
    engine.runAndWait()

def record(count):
    freq = 44100

    # 녹음 제한 시간: 5초
    duration = 5
    print('말씀해주세요')
    recording = sd.rec(int(duration*freq), samplerate=freq, channels=1)
    # duration 동안 대기
    sd.wait()

    audio_file = f'recording{count}.wav'
    wv.write(audio_file, recording, freq, sampwidth=2)

    return audio_file

def speech_recognition(model, audio_file):
    audio = whisper.load_audio(audio_file)
    audio = whisper.pad_or_trim(audio)
    mel = whisper.log_mel_spectrogram(audio).to(model.device)

    _, probs = model.detect_language(mel)
    print(f"Detected language: {max(probs, key=probs.get)}")
    # 한국어만 인식할 수 있도록 처리
    # 한국어가 아닌 걸로 인식하면 다시 말해야 함
    if max(probs, key=probs.get) == 'ko':
        result = model.transcribe(audio_file)
    else:
        return 

    return result['text']


def chatting(client, user_query, parking_rate, max_cap,current_cars):
    response = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[
            {"role": "system", "content": f'종료라고 하면 긍정적인 대답해줘 \
                주차장 요금은 초당 {parking_rate}원이야 주차장에 최대 수용 대수는 {max_cap}대야 \
                    현재 주차장에는 {current_cars}대가 있어'},
            {"role": "user", "content": user_query}
        ],
        max_tokens= 200,
    )
    
    return response.choices[0].message.content


def speech_to_gpt(model, client):
    # get datas from DB
    parking_rate = 1
    max_cap = 10
    current_cars = 3

    record_count = 0

    while True:
        print('start')
        audio_file = record(record_count)
        text = speech_recognition(model, audio_file)
        print(text)

        if text is None or len(text)<2:
            print('다시 한 번 말씀해주세요')
            speak('다시 한 번 말씀해주세요')
            continue
        else:
            response = chatting(client, text, parking_rate, max_cap, current_cars)
            print(response)
            speak(response)
            if '그만' in text or '종료' in text:
                print('프로그램 종료')
                speak('프로그램 종료')
                return

        record_count+=1

if __name__ == '__main__':
    model = whisper.load_model('small')
    client = OpenAI(api_key='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')
    start_voice_interface('10.10.59.237', 2000)
    speech_to_gpt(model, client)
