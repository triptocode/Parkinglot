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
def speak(answer:str):
    engine = pyttsx3.init()
    engine.say(answer)
    engine.runAndWait()

def record(count):
    freq = 44100

    # 녹음 제한 시간: 5초
    duration = 5

    print('말씀해주세요')
    recording = sd.rec(int(duration*freq), samplerate=freq, channels=2)
    
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

def chatting(client, user_query):
    response = client.chat.completions.create(
        model="gpt-3.5-turbo",
        messages=[
            {"role": "system", "content": '종료라고 하면 긍정적인 대답해줘 \
                주차장 요금은 초당 100원이야 주차장에 최대 수용 대수는 2대야 \
                    현재 주차장에는 0대가 있어'},
            {"role": "user", "content": user_query}
        ],
        max_tokens= 200,
    )
    
    return response.choices[0].message.content
    


def main():
    
    record_count = 0

    print('대화를 시작하려면 1을 눌러주세요. 종료하시려면 2를 눌러주세요.')
    speak('대화를 시작하려면 1을 눌러주세요. 종료하시려면 2를 눌러주세요.')
    start = input()

    if start == '2':
        print('프로그램 종료')
        speak('프로그램 종료')
        return
    
    else:
        model = whisper.load_model('base')
        client = OpenAI(api_key='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')

        while True:
            
            audio_file = record(record_count)
            text = speech_recognition(model, audio_file)

            print(text)
            if text is None or len(text)<2:
                print('다시 한 번 말씀해주세요')
                speak('다시 한 번 말씀해주세요')
                continue
            else:
                response = chatting(client, text)
                print(response)
                speak(response)
                if '그만' in text or '종료' in text:
                    print('프로그램 종료')
                    speak('프로그램 종료')
                    return

            record_count+=1

if __name__ == '__main__':
    main()
