import uart
from queue import Queue
import threading
from license_plate import license_detect, license_to_string
import re
import db_com
import datetime
import rapi_lcd
import smbus
import time
from pymongo import MongoClient
def uart_rx(rq):
    while True:
        msg = uart.uart_one_receive()
        rq.put(msg)

def uart_tx(tq):
    while True:
        msg  = tq.get()
        uart.uart_transmit(msg)

def check_pattern(s):
    pattern = r'^\d{3}[A-Za-z]\d{4}$'
    return bool(re.match(pattern, s))


# 디바이스 매개변수 정의
I2C_ADDR  = 0x27 # I2C 디바이스 주소
LCD_WIDTH = 16   # 한 줄당 최대 문자 수

# 디바이스 상수 정의
LCD_CHR = 1 # 모드 - 데이터 전송
LCD_CMD = 0 # 모드 - 명령 전송

LCD_LINE_1 = 0x80 # LCD의 첫 번째 줄을 위한 RAM 주소
LCD_LINE_2 = 0xC0 # LCD의 두 번째 줄을 위한 RAM 주소
LCD_LINE_3 = 0x94 # LCD의 세 번째 줄을 위한 RAM 주소
LCD_LINE_4 = 0xD4 # LCD의 네 번째 줄을 위한 RAM 주소

LCD_BACKLIGHT  = 0x08  # 켜짐
#LCD_BACKLIGHT = 0x00  # 꺼짐

ENABLE = 0b00000100 # Enable 비트

# 타이밍 상수
E_PULSE = 0.0005
E_DELAY = 0.0005

# I2C 인터페이스 열기
#bus = smbus.SMBus(0)  # Rev 1 Pi는 0을 사용
bus = smbus.SMBus(1) # Rev 2 Pi는 1을 사용

'''DB'''
host = '10.10.59.223' # host ip : db server
port = 27017
username = 'rpi'
password = '1234'   

client = MongoClient(host=host, port=port, username=username, password=password)

db_name = 'parkinglot'
col_name = 'license'

# 현재 시간을 YYYYmmddHHMMSS 형식으로 얻는 방법환
current_time = datetime.datetime.today()
current_time = current_time.strftime('%Y%m%d%H%M%S')


'''communication thered'''
rq = Queue() # uart receive queue
tq = Queue() # uart transmit queue

r_thread = threading.Thread(target=uart_rx,args=(rq,))
t_thread = threading.Thread(target=uart_tx,args=(tq,))

r_thread.start()
t_thread.start()

'''lcd'''
rapi_lcd.lcd_init()

'''init'''
cnt =0
cam_number = 0
location  = {0:'I', 2:'O'}
position = {'I':0, 'O':2}

while True:
    msg=rq.get()
    #if sentence
    transmit =""
    print("msg:",msg)
    if msg == 'I' or msg == 'O':
        cam_number = position[msg]
        
        license_plates,frame=license_detect(cam_number)
        number=license_to_string(license_plates,frame)
        
        if number is None or not check_pattern(number):
            transmit = location[cam_number]+"AIL"+(" "*17)
            print("transmit",transmit)
            print("transmit_len:",len(transmit))
            tq.put(transmit)
        else:
            transmit = location[cam_number]+"ASS "+number
            
            if location[cam_number]=='I':
                current_time = datetime.datetime.today()
                current_time = current_time.strftime('%Y%m%d%H%M%S')
                result_In = db_com.insert_car_info(number, current_time)
                if result_In==False:
                    transmit = location[cam_number]+"AIL"+(" "*17)
                else:
                    transmit = transmit+(" "*8)
            elif location[cam_number]=='O':
                result_Out = db_com.delete_car_info(number)
                if result_Out == False:
                    transmit = location[cam_number]+"AIL"+(" "*17)
                else:
                    fee = result_Out[1]
                    charge = str(fee)+"WON"
                    transmit_charge = charge+" "*(7-len(charge))
                    print("transmit_charge:",transmit_charge)
                    print("transmit_charge length:",len(transmit_charge))
                    transmit= transmit+" "+transmit_charge
            print("transmit:",transmit)
            print("transmit_len:",len(transmit))
            tq.put(transmit)
        