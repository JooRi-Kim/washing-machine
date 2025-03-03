## 📌 프로젝트 소개 - IoT 기반 세탁기 시스템
- 다양한 센터 및 모터를 활용하여 세탁기 시스템을 구현하였습니다.


---


## 📅 개발 기간
- 24/12/01 ~ 24/12/24


---


## 👨‍💻 개발 담당
- 기능블록도 설계
- 세탁 모드 선택 기능 구현
- 각 모드에 따른 예상 세탁 시간 출력 기능 구현
- 현재 세탁 상태 실시간 표시 기능 구현


---


## 🛠️ 사용 기술 및 하드웨어
- 프로그래밍 언어: C (wiringPi 라이브러리 활용)
- 마이크로컨트롤러: 라즈베리파이3
- 하드웨어(센서 및 모터)
  - 키패드 1개 
  - 서보 모터 1개
  - DC 모터 2개
  - 모터 드라이버 1개
  - Text LCD 1개
  - Buzzer 1개
  - 초음파 센서 1개
  - 워터펌프 2개
  - 브레드보드, 전원공급장치


---


## 🎯 구현 기능
- 세탁 모드 선택 (키패드 입력 처리 - 표준/강력/섬세/탈수)
- 각 모드에 따른 예상 세탁 시간 표시 (키패드로 모드 변경 -> Text LCD 출력)
- 현재 세탁 상태 실시간 표시 (Text LCD 출력)
- 세탁 시작 시 자동 문 닫힘, 종료 시 문 열림 (서보 모터)
- 세탁 시작 시 자동 물 공급, 종료 시 물 배출 (워터펌프 제어)
- 세탁 단계에 따른 세탁기 회전 속도 및 방향 제어(DC 모터)
- 물이 부족하거나 과도할 경우 경고음과 메시지 알림 (Buzzer, Text LCD)


---


## 📊 기능블록도
![기능 블록도](https://private-user-images.githubusercontent.com/122363990/418546220-5e922c63-8ee1-444c-8bc9-6515f3cefcf5.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NDEwMDAzOTYsIm5iZiI6MTc0MTAwMDA5NiwicGF0aCI6Ii8xMjIzNjM5OTAvNDE4NTQ2MjIwLTVlOTIyYzYzLThlZTEtNDQ0Yy04YmM5LTY1MTVmM2NlZmNmNS5wbmc_WC1BbXotQWxnb3JpdGhtPUFXUzQtSE1BQy1TSEEyNTYmWC1BbXotQ3JlZGVudGlhbD1BS0lBVkNPRFlMU0E1M1BRSzRaQSUyRjIwMjUwMzAzJTJGdXMtZWFzdC0xJTJGczMlMkZhd3M0X3JlcXVlc3QmWC1BbXotRGF0ZT0yMDI1MDMwM1QxMTA4MTZaJlgtQW16LUV4cGlyZXM9MzAwJlgtQW16LVNpZ25hdHVyZT0xZWQ4ZjE1MzI0MjY0N2Q1NjM1YjQ3OWE4MDRhOTBmMjA3MGEzNGM2MWRjOThlMGZhNjg4MjcxNjI4YTI0OGQ3JlgtQW16LVNpZ25lZEhlYWRlcnM9aG9zdCJ9.3FEO3Dd3MXiOSi23fhqNi1DCLsgYcQKQQIGsAs_hsR8)


---


## 📂 프로젝트 구조
```
📂IoT_Washing_Machine/
├──📂src/
│   ├──📂iot_washing_machine.c  # 메인 코드
│   ├──📂 motor_control.c  # 모터 제어 코드
│   ├──📂 lcd_display.c  # LCD 출력 코드
│   ├──📂 ultrasonic_sensor.c  # 초음파 센서 측정 코드
│   ├──📂 buzzer.c  # 부저 제어 코드
│   ├──📂 servo_motor.c  # 서보 모터 제어 코드
│   ├──📂 keypad.c  # 키패드 입력 처리 코드
│   └──📂 Makefile
```


---


## 📷 세탁기 전면
![세탁기 전](https://private-user-images.githubusercontent.com/122363990/418546274-f3077d20-78e9-4991-95b7-33c419efa86f.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NDEwMDA0MDcsIm5iZiI6MTc0MTAwMDEwNywicGF0aCI6Ii8xMjIzNjM5OTAvNDE4NTQ2Mjc0LWYzMDc3ZDIwLTc4ZTktNDk5MS05NWI3LTMzYzQxOWVmYTg2Zi5wbmc_WC1BbXotQWxnb3JpdGhtPUFXUzQtSE1BQy1TSEEyNTYmWC1BbXotQ3JlZGVudGlhbD1BS0lBVkNPRFlMU0E1M1BRSzRaQSUyRjIwMjUwMzAzJTJGdXMtZWFzdC0xJTJGczMlMkZhd3M0X3JlcXVlc3QmWC1BbXotRGF0ZT0yMDI1MDMwM1QxMTA4MjdaJlgtQW16LUV4cGlyZXM9MzAwJlgtQW16LVNpZ25hdHVyZT0yMzY1ZWNmNTU2M2I4ZDYzMzMwMjYzZDljYjI3Mjk1NzU2MTMwYjUxNTQzZjUwMWU0MGQxOThjZTA5YzQ1NmQwJlgtQW16LVNpZ25lZEhlYWRlcnM9aG9zdCJ9.t3myjZoZlfHOH3j1VcIFJRXHCdX6LVA1hV2a8SEX7CY)


---


## 🔗 블로그: 프로젝트 관련 포스트
- [YOLOv8 학습 및 모델 적용 과정](https://djjin02.tistory.com/205)
- [객체 탐지 웹 애플리케이션 구현](https://djjin02.tistory.com/207)


---

