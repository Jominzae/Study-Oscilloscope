# STM32 주변장치 인터페이스 및 통신 프로토콜 분석

### 개요 
* STM32 마이크로컨트롤러를 활용하여 다양한 통신 프로토콜(UART, I2C)과 센서(온습도, 초음파)의 동작 원리를 파악하고 오실로스코프를 통해 실제 파형을 분석

## 1. STM32 - UART 오실로스코프 파형 분석
* 목표 : UART 통신 설정값에 따른 실제 데이터 송신 파형 분석
* 설정 : Baudrate 9600 bps, Data 8 bit, Stop 1 bit, Idle State HIGH
* 분석 내용 : 문자열 "4601" (Hex: 0x34, 0x36, 0x30, 0x31)의 이진 데이터 분석 및 LSB 우선 전송 확인

<div align="center">
<img width="816" height="615" alt="image" src="https://github.com/user-attachments/assets/fb1fa4f8-e22b-4ece-8059-5a8725d1f1df" />
</div>

## 2. DHT11 온습도 센서 GPIO 파형 분석
* 목표 : 단일선(Single-wire) 통신을 사용하는 DHT11 센서의 데이터 프레임 분석
* 분석 결과:
  - 습도(33.0%)와 온도(29.8C) 데이터가 이진수로 어떻게 표현되는지 오실로스코프로 측정
  - Humi High/Low, Temp High/Low, 그리고 데이터 무결성을 위한 Checksum(체크섬) 검증 완료
  - 데이터시트를 직접 분석하여 프로토콜 분석 완료
 
<div align="center">
<img width="814" height="614" alt="image" src="https://github.com/user-attachments/assets/6dbb4b03-bd47-461f-a102-6b512c0951f0" />
<img width="1086" height="445" alt="image" src="https://github.com/user-attachments/assets/97d8833a-667a-4324-a8fb-043df4745a69" />
</div>


## 3. 초음파 센서 거리에 따른 펄스 측정
* 목표 : 초음파 센서의 Trig 신호와 Echo 신호 간의 시간 차를 이용한 거리 계산
* 측정 결과 :
    - Trig 신호(10us) 입력 후 Echo 신호의 길이를 측정하여 거리 산출
    - Echo 230us 측정 시 약 3cm, Echo 950us 측정 시 약 16cm로 계산되어 실제 거리와 일치함을 확인

<div align="center">
<img width="1249" height="937" alt="image" src="https://github.com/user-attachments/assets/3b2e66dc-5bee-4414-9ff6-b6b3119c2171" />
</div>


## 4. I2C 프로토콜 분석 및 LCD 제어
* 목표 : I2C 통신의 시작/종료 조건 및 데이터 전송 절차 이해.
* 분석 내용 :
    - SDA(데이터)와 SCL(클럭) 선의 동작 원리 및 슬레이브 주소(0x27) 전송 과정 분석
    - 데이터 '2'(0x32)를 전송할 때의 Write Bit와 ACK Bit 발생 확인
    - I2C – LCD – STM32 배선도 kiCad를 이용하여 작성
 
<div align="center">
<img width="1245" height="930" alt="image" src="https://github.com/user-attachments/assets/7c8a5dfd-366a-4758-8370-6b9ee74e3112" />
<img width="1247" height="927" alt="image" src="https://github.com/user-attachments/assets/4ffbcf9e-458b-453f-bee6-3b12fe84e921" />
<img width="1236" height="923" alt="image" src="https://github.com/user-attachments/assets/5390da5c-adcd-4d19-b462-450aa163f4d6" />
</div>
