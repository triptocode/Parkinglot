import RPi.GPIO as GPIO
import serial
import time

ser = serial.Serial(
    port='/dev/ttyAMA2',
    baudrate=115200
    )


def uart_receive():
    '''STM32에게서 UART로 데이터를 수신 받는 함수'''
    response = ser.readline()
    msg = response.decode('utf-8')
    if ord(msg[0])==0:
        msg = msg[1:]
    return msg[:-1]


def uart_one_receive():
    '''STM32에게서 UART로 데이터를 하나만 수신 받는 함수'''
    response = ser.read(1)
    msg = response.decode('utf-8')
    return msg


def uart_transmit(message: str):
    '''STM32에게 UART로 데이터를 송신 하는 함수'''
    # print("uart_transmit:")
    for i in message:
        ser.write(i.encode())
        # print(i,end='')
    ser.write('\n'.encode())
    # print(ord('\n'))


def uart_rx(rq):
    '''STM32로부터 받은 데이터를 큐에 넣는 함수. 스레딩용'''
    while True:
        msg = uart_one_receive()
        rq.put(msg)


def uart_tx(tq):
    '''STM32에 보낼 데이터를 큐에 넣는 함수. 스레딩용'''
    while True:
        msg  = tq.get()
        uart_transmit(msg)
