#!/usr/bin/python
import smbus
import time

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

def lcd_init():
  # 디스플레이 초기화
  lcd_byte(0x33,LCD_CMD) # 110011 초기화
  lcd_byte(0x32,LCD_CMD) # 110010 초기화
  lcd_byte(0x06,LCD_CMD) # 000110 커서 이동 방향
  lcd_byte(0x0C,LCD_CMD) # 001100 디스플레이 켜기, 커서 끄기, 깜빡임 끄기 
  lcd_byte(0x28,LCD_CMD) # 101000 데이터 길이, 줄 수, 폰트 크기
  lcd_byte(0x01,LCD_CMD) # 000001 디스플레이 지우기
  time.sleep(E_DELAY)

def lcd_byte(bits, mode):
  # 데이터 핀에 바이트 전송
  # bits = 데이터
  # mode = 1 이면 데이터
  #        0 이면 명령

  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT
  bits_low = mode | ((bits<<4) & 0xF0) | LCD_BACKLIGHT

  # 상위 비트
  bus.write_byte(I2C_ADDR, bits_high)
  lcd_toggle_enable(bits_high)

  # 하위 비트
  bus.write_byte(I2C_ADDR, bits_low)
  lcd_toggle_enable(bits_low)

def lcd_toggle_enable(bits):
  # Enable 토글
  time.sleep(E_DELAY)
  bus.write_byte(I2C_ADDR, (bits | ENABLE))
  time.sleep(E_PULSE)
  bus.write_byte(I2C_ADDR,(bits & ~ENABLE))
  time.sleep(E_DELAY)

def lcd_string(message,line):
  # 디스플레이에 문자열 전송

  message = message.ljust(LCD_WIDTH," ")

  lcd_byte(line, LCD_CMD)

  for i in range(LCD_WIDTH):
    lcd_byte(ord(message[i]),LCD_CHR)

def main():
  # 메인 프로그램 블록

  # 디스플레이 초기화
  lcd_init()

  while True:

    # 텍스트 전송
    lcd_string("RPiSpy         <",LCD_LINE_1)
    lcd_string("I2C LCD        <",LCD_LINE_2)

    time.sleep(3)
    lcd_byte(0x01, LCD_CMD)
    time.sleep(1)

if __name__ == '__main__':
  try:
    main()
  except KeyboardInterrupt:
    pass
  finally:
    lcd_byte(0x01, LCD_CMD)
