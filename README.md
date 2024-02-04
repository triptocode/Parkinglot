# Project ABC

* 이제까지 배운 AI, 임베디드 기술을 총동원하여 주차장 시스템을 구축해내는 프로젝트.

## High Level Design

* ![./doc/high_level_design.png](./doc/high_level_design.png)

물체가 다가오면 초음파센서로 인지하여 라즈베리파이로 통신를 보낸다.
통신을 받은 라즈베리파이는 웹캠으로 물체를 촬영하고
차량이면 번호판을 OCR로 읽어낸다.
DB에 차량번호와 시간을 보내고
STM에게 열어도 좋다는 통신을 보낸다.
통신을 받은 STM은 서보모터를 작동하여 차단기를 올린다.

물체가 출구에 도착했을 때도 위와 같이 작동하되,
DB와의 통신으로 해당 차량이 얼마나 오랫동안 주차장에 있었는지를 계산하여 요금을 청구한다.
요금이 청구되면 차단기를 올려준다.

## Clone code

```shell
git clone https://github.com/HardCoding0417/Parking-lot-project
```

* 리눅스에서 cli git을 사용하고 있다면 아래의 방법으로 clone한다.

```shell
gh repo clone HardCoding0417/Parking-lot-project
```

## Prerequite

```shell
python -m venv .venv
source .venv/bin/activate
sudo apt install tesseract-ocr
pip install -r requirements.txt
```
* requirements.txt로 설치가 잘 되지 않는다면
pymongo
sounddevice 
wavio
whisper
pyttsx3
ultralytics
cv2
pytesseract
pytesseract-ocr
을 개별적으로 설치해본다.

## Steps to build

* 라즈베리파이와 STM32 사이의 통신을 위한 UART설정이 필요하다.

```
Embedded part.

clock configuration
: 100MHz

1. Ultrasonic set
ultra 1's TIM2 trigger pin : GPIOA8
ulrta 1's TIM2 echo pin : PA0
ultra 2's TIM2 trigger pin : GPIOB5
ulrta 2's TIM2 echo pin : PB10

Channel : 1, 3
Prescale : 100-1
Counter period : 0xffff-1
: HC-SR04 초음파센서의 Datasheet상에서 10us의 TTL값이 필요하기 때문에 Parameter Setting을 위처럼 하였음

2. Servo motor set
TIM3 Channel pin 1 : PC6
TIM3 Channel pin 2 : PA7

Channel : 1, 2 
Prescale : 2000-1
Counter period : 1000-1
: SG-90 서보모터에서 20ms(50hz)의 값이 필요하기 때문에 Parameter Setting을 위처럼 하였음

3. UART set
UART1
Bluetooth Tx pin : PA9
Bluetooth Rx pin : PA10
: Serial Bluetooth Terminal(APP)사용하여 관리자(Manager)에게 log 기록 전송

UART6
MCU Communication Tx : PA11
MCU Communication Rx : PC7
: Serial로 RaspberryPi와 UART통신을 통해 Data를 Receive / Transmit

4. LCD set
SCL pin : PB6
SDA pin : PB7
: LCD 모듈을 사용하기 위하여 SCL, SDA 핀 사용

5. LED set
Green LED : PC8
Red LED : PA5
: 열렸다면 Green, 닫힌다면 Red

6. RCCAR set
IN1 : 22
IN2 : 23
IN3 : 17
IN4 : 18
ENA : 12
ENB : 13
Bluetooth Tx (UART) : 14
Bluetooth Rx (UART) : 15
: 번호판을 붙이고 주차장에 진입할 차량 구현

7. Task add
Task 1 : LCD
Task 2 : 초음파
Task 3 : 블루투스 통신 , LED
Task 4 : 서보모터
Task 5 : MCU 통신

```

## Steps to run

* (프로젝트 실행방법에 대해서 기술, 특별한 사용방법이 있다면 같이 기술)

```shell
Embedded part.

1. RTOS
페리페럴 여러 개를 동시에 작동시키기 위해서 Binary Semaphore 방식으로 Task 5개 생성

2. UART
라즈베리파이와 STM이 통신하기 위해 UART 설정

3. I2C
UART 통신으로 전송받은 데이터를 LCD에 출력해주기 위해 I2C 통신 설정

```

## Output

* (프로젝트 실행 화면 캡쳐)

![./result.jpg](./result.jpg)

## Appendix

* (참고 자료 및신 알아두어야할 사항들 기술)
