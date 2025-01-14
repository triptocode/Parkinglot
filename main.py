from pymongo import MongoClient
import uart
from queue import Queue
import threading
from license_plate import license_detect, license_to_string
import re
import db_com
import datetime
import socket

def transmit_order(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        client_socket.connect((host, port))
        print(f"서버에 연결됨: {host}:{port}")
        # 서버에 메세지 보내기
        client_socket.sendall(b'1')  # 문자 '1'을 바이트로 전송
        print(f"transmit order")

def check_pattern(s):
    '''전달 받은 데이터가 번호판 양식에 맞는지 체크해주는 함수'''
    pattern = r'^\d{3}[A-Za-z]\d{4}$'
    return bool(re.match(pattern, s))

# DB
db_host = '10.10.59.223'  # host ip : db server
db_port = 27017
username = 'rpi'
password = '1234'

client = MongoClient(host=db_host,
                     port=db_port,
                     username=username,
                     password=password)

db_name = 'parkinglot'
col_name = 'license'

# 현재 시간을 YYYYmmddHHMMSS 형식으로 얻는 방법환
current_time = datetime.datetime.today()
current_time = current_time.strftime('%Y%m%d%H%M%S')


# UART를 위한 큐와 스레딩
rq = Queue()  # uart receive queue
tq = Queue()  # uart transmit queue
r_thread = threading.Thread(target=uart.uart_rx, args=(rq,))
t_thread = threading.Thread(target=uart.uart_tx, args=(tq,))
r_thread.start()
t_thread.start()

# init for voice interface
voice_host = '10.10.59.55'
voice_port = 2000

# 초기화
cnt =0
cam_number = 0
location = {0: 'I', 2: 'O'}
position = {'I': 0, 'O': 2}
parking_pee_per_sec = 100

while True:
    # 수신 큐로부터 메세지를 꺼낸다.
    msg = rq.get()
    transmit_order(voice_host, voice_port)
    transmit = ""
    print("msg:", msg)
    # I나 O를 받기로 약속했다. I는 입차상황, O는 출차상황임. 잘못된 메세지는 거른다.
    if msg == 'I' or msg == 'O':
        # 입차 시에는 입구 카메라, 출차 시에는 출차 카메라를 작동하여 번호판을 디텍트한다.
        cam_number = position[msg]
        license_plates, frame = license_detect(cam_number)
        number = license_to_string(license_plates, frame)

        # 정규식으로 번호판이 맞는지 체크하여, 번호판이 아니면 I나 O에 AIL을 붙여서 오류메세지를 보낸다.
        if number is None or not check_pattern(number):
            transmit = location[cam_number]+"AIL"+(" "*17)
            print("transmit", transmit)
            print("transmit_len:", len(transmit))
            tq.put(transmit)

        # 번호판이 맞다면 I나 O에 ASS를 붙여서 보낸다.
        else:
            transmit = location[cam_number]+"ASS "+number
            # 입차상황이면 DB에 입차 데이터를 넣는다.
            if location[cam_number] == 'I':
                current_time = datetime.datetime.today()
                current_time = current_time.strftime('%Y%m%d%H%M%S')
                result_In = db_com.insert_car_info(number, current_time)

                # DB에 차량 데이터가 들어가지 않았을 경우에 대한 예외처리
                if result_In == False:
                    transmit = location[cam_number]+"AIL"+(" "*17)
                else:
                    # 송신할 데이터를 준비
                    transmit = transmit+(" "*8)


            # 출차 상황이면 DB에 출차 데이터를 넣는다.
            elif location[cam_number]=='O':
                result_out = db_com.delete_car_info(number)
                # DB에 차량 데이터가 들어가지 않았을 경우에 대한 예외처리
                if result_out == False:
                    transmit = location[cam_number]+"AIL"+(" "*17)
                else:
                    # 송신할 데이터를 준비.
                    fee = result_out[1]
                    charge = str(fee)+"WON"
                    transmit_charge = charge+" "*(7-len(charge))
                    print("transmit_charge:",transmit_charge)
                    print("transmit_charge length:",len(transmit_charge))
                    transmit = transmit+" "+transmit_charge

            print("transmit:",transmit)
            print("transmit_len:",len(transmit))

            # 데이터를 송신한다.
            tq.put(transmit)
