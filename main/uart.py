import RPi.GPIO as GPIO
import serial
import time
ser = serial.Serial(
    port = '/dev/ttyAMA2',
    baudrate=115200
    )

def uart_receive():
    response = ser.readline()
    msg = response.decode('utf-8')
    if ord(msg[0])==0:
        msg = msg[1:]
    return msg[:-1]

def uart_one_receive():
    response  = ser.read(1)
    msg = response.decode('utf-8')
    return msg

def uart_transmit(message:str):
    #print("uart_transmit:")
    for i in message:
        ser.write(i.encode())
        #print(i,end='')
    ser.write('\n'.encode())
    #print(ord('\n'))


