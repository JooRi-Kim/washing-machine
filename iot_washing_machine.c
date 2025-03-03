#include <stdio.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softTone.h>
#include <softPwm.h>

// I2C LCD 설정
#define LCD_ADDR 0x27
#define LCD_BACKLIGHT 0x08
#define ENABLE 0x04
#define RS 0x01
#define BUZZER_PIN 4
#define SERVO_PIN 24 // 서보모터 핀 설정

// 키패드 핀 설정
#define R1 5
#define R2 6
#define R3 13
#define R4 19
#define C1 12
#define C2 16
#define C3 20
#define C4 21

#define TRIG_PIN 8  // 초음파 센서 TRIG 핀
#define ECHO_PIN 7  // 초음파 센서 ECHO 핀

// DC 모터 핀 설정
#define MOTOR_MT_P_PIN 10
#define MOTOR_MT_N_PIN 9
#define LEFT_ROTATE 1
#define RIGHT_ROTATE 2

//워터펌프1
#define WATER_IN_P_PIN 25
#define WATER_IN_N_PIN 11

//워터펌프2
#define WATER_OUT_P_PIN 27
#define WATER_OUT_N_PIN 22

// 키패드 매핑
char keyMap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

int lcdFd;
int menuIndex = 0; // 현재 선택된 메뉴 인덱스

// 메뉴 항목 정의
const char* menuItems[] = {
    "1. Standard",
    "2. Strong",
    "3. Delicate",
    "4. Dhydration"
};
const int MENU_COUNT = 4; // 메뉴 항목 수

// 함수 선언
void displayMenu();         // 메뉴를 표시하는 함수
void rotateMotor(int direction); // 모터를 회전시키는 함수
void rotateMotor2(int direction); // 모터를 반복적으로 회전시키는 함수
void rotateMotor3(int direction); // 모터의 방향을 전환하며 회전시키는 함수
void rotateMotor4(int direction); // 섬세 모드에서 모터를 회전시키는 함수
void rotateMotor5(int direction); // 강력 모드에서 모터를 회전시키는 함수
void rotateMotor6(int direction); // 탈수 모드에서 모터를 회전시키는 함수
void waterinMotor(int direction); // 워터펌프 입력 모터 제어 함수
void wateroutMotor(int direction);// 워터펌프 출력 모터 제어 함수
void servoMoveToAngle(int angle); // 서보모터 각도를 설정하는 함수
void keypadInit();          // 키패드를 초기화하는 함수
char getKey();              // 키패드의 키 입력을 감지하는 함수
void runStandardCycle();    // 표준 세탁 사이클 실행 함수
void runStrongCycle();      // 강력 세탁 사이클 실행 함수
void runDelicateCycle();    // 섬세 세탁 사이클 실행 함수
void runDehydrationCycle(); // 탈수 사이클 실행 함수
void selectMenu();          // 선택된 메뉴 실행 함수

// 초음파 센서로 거리 측정
float getWaterLevel() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    while (digitalRead(ECHO_PIN) == LOW);
    long startTime = micros();
    while (digitalRead(ECHO_PIN) == HIGH);
    long travelTime = micros() - startTime;

    // 거리 계산 (단위: cm)
    float distance = travelTime / 58.0;
    return distance;
}

// LCD 명령 전송
void lcdSendCommand(int cmd) {
    int highNibble = cmd & 0xF0;
    int lowNibble = (cmd << 4) & 0xF0;

    wiringPiI2CWrite(lcdFd, highNibble | LCD_BACKLIGHT);
    wiringPiI2CWrite(lcdFd, highNibble | ENABLE | LCD_BACKLIGHT);
    delay(1);
    wiringPiI2CWrite(lcdFd, highNibble & ~ENABLE | LCD_BACKLIGHT);

    wiringPiI2CWrite(lcdFd, lowNibble | LCD_BACKLIGHT);
    wiringPiI2CWrite(lcdFd, lowNibble | ENABLE | LCD_BACKLIGHT);
    delay(1);
    wiringPiI2CWrite(lcdFd, lowNibble & ~ENABLE | LCD_BACKLIGHT);
}

// LCD 데이터 전송
void lcdSendData(int data) {
    int highNibble = data & 0xF0;
    int lowNibble = (data << 4) & 0xF0;

    wiringPiI2CWrite(lcdFd, highNibble | RS | LCD_BACKLIGHT);
    wiringPiI2CWrite(lcdFd, highNibble | ENABLE | RS | LCD_BACKLIGHT);
    delay(1);
    wiringPiI2CWrite(lcdFd, highNibble & ~ENABLE | RS | LCD_BACKLIGHT);

    wiringPiI2CWrite(lcdFd, lowNibble | RS | LCD_BACKLIGHT);
    wiringPiI2CWrite(lcdFd, lowNibble | ENABLE | RS | LCD_BACKLIGHT);
    delay(1);
    wiringPiI2CWrite(lcdFd, lowNibble & ~ENABLE | RS | LCD_BACKLIGHT);
}
void keypadInit() {
    pinMode(R1, OUTPUT);
    pinMode(R2, OUTPUT);
    pinMode(R3, OUTPUT);
    pinMode(R4, OUTPUT);
    pinMode(C1, INPUT);
    pinMode(C2, INPUT);
    pinMode(C3, INPUT);
    pinMode(C4, INPUT);
    pullUpDnControl(C1, PUD_UP);
    pullUpDnControl(C2, PUD_UP);
    pullUpDnControl(C3, PUD_UP);
    pullUpDnControl(C4, PUD_UP);
}

// 키패드 입력 처리
char getKey() {
    int rowPins[4] = { R1, R2, R3, R4 };
    int colPins[4] = { C1, C2, C3, C4 };

    for (int row = 0; row < 4; row++) {
        digitalWrite(rowPins[row], LOW);
        for (int col = 0; col < 4; col++) {
            if (digitalRead(colPins[col]) == LOW) {
                delay(200); // 디바운싱
                while (digitalRead(colPins[col]) == LOW);
                digitalWrite(rowPins[row], HIGH);
                return keyMap[row][col];
            }
        }
        digitalWrite(rowPins[row], HIGH);
    }
    return '\0'; // 입력 없음
}

// LCD 초기화
void lcdInit() {
    lcdSendCommand(0x33);
    lcdSendCommand(0x32);
    lcdSendCommand(0x28);
    lcdSendCommand(0x0C);
    lcdSendCommand(0x06);
    lcdSendCommand(0x01);
    delay(5); // 초기화 후 지연
}

// LCD 커서 설정
void lcdSetCursor(int row, int col) {
    int addr = (row == 0 ? 0x80 : 0xC0) + col;
    lcdSendCommand(addr);
}

// LCD 메시지 출력
void lcdPrint(const char* message) {
    while (*message) {
        lcdSendData(*message++);
    }
}

// 워터모터1 제어
void waterinMotor(int direction) {
    if (direction == LEFT_ROTATE) {
        digitalWrite(WATER_IN_P_PIN, HIGH);
        digitalWrite(WATER_IN_N_PIN, LOW);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(WATER_IN_P_PIN, LOW);
        digitalWrite(WATER_IN_N_PIN, HIGH);
    }
    delay(20000); // 20초 동안 회전
    digitalWrite(WATER_IN_P_PIN, LOW); // 모터 정지
    digitalWrite(WATER_IN_N_PIN, LOW);
}

// 워터모터2 제어
void wateroutMotor(int direction) {
    if (direction == LEFT_ROTATE) {
        digitalWrite(WATER_OUT_P_PIN, HIGH);
        digitalWrite(WATER_OUT_N_PIN, LOW);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(WATER_OUT_P_PIN, LOW);
        digitalWrite(WATER_OUT_N_PIN, HIGH);
    }
    delay(20000); // 20초 동안 회전
    digitalWrite(WATER_OUT_P_PIN, LOW); // 모터 정지
    digitalWrite(WATER_OUT_N_PIN, LOW);
}

// DC 모터 회전 제어
void rotateMotor(int direction) {
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    delay(10000); // 10초 동안 회전
    digitalWrite(MOTOR_MT_P_PIN, LOW); // 모터 정지
    digitalWrite(MOTOR_MT_N_PIN, LOW);
}

// DC 모터 회전 제어 멈춤 동작 반복
void rotateMotor2(int direction) {
    for (int i = 0; i < 5; i++) { // 5번 반복
        if (direction == LEFT_ROTATE) {
            digitalWrite(MOTOR_MT_P_PIN, HIGH);
            digitalWrite(MOTOR_MT_N_PIN, LOW);
        }
        else if (direction == RIGHT_ROTATE) {
            digitalWrite(MOTOR_MT_P_PIN, LOW);
            digitalWrite(MOTOR_MT_N_PIN, HIGH);
        }

        delay(1000); // 1.5초 동안 회전

        // 모터 정지
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, LOW);

        delay(1000); // 0.5초 동안 정지
    }
}

// DC 모터 회전 제어 방향전환
void rotateMotor3(int direction) {
    // 첫 번째 방향으로 4초 동안 회전
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    delay(4000); // 4초 동안 회전

    // 모터 정지
    digitalWrite(MOTOR_MT_P_PIN, LOW);
    digitalWrite(MOTOR_MT_N_PIN, LOW);
    delay(2000); // 2초 동안 정지

    // 반대 방향으로 4초 동안 회전
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    delay(4000); // 4초 동안 회전

    // 모터 정지
    digitalWrite(MOTOR_MT_P_PIN, LOW);
    digitalWrite(MOTOR_MT_N_PIN, LOW);
}

// DC 모터 회전 제어 방향전환 섬세모드
void rotateMotor4(int direction) {
    // 첫 번째 방향으로 4초 동안 회전
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    delay(4000); // 4초 동안 회전

    // 모터 정지
    digitalWrite(MOTOR_MT_P_PIN, LOW);
    digitalWrite(MOTOR_MT_N_PIN, LOW);
    delay(2000); // 2초 동안 정지

    // 반대 방향으로 4초 동안 회전
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    delay(4000); // 4초 동안 회전

    // 모터 정지
    digitalWrite(MOTOR_MT_P_PIN, LOW);
    digitalWrite(MOTOR_MT_N_PIN, LOW);

    // 첫 번째 방향으로 4초 동안 회전
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    delay(4000); // 4초 동안 회전

    // 모터 정지
    digitalWrite(MOTOR_MT_P_PIN, LOW);
    digitalWrite(MOTOR_MT_N_PIN, LOW);
    delay(2000); // 2초 동안 정지

    // 반대 방향으로 4초 동안 회전
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    delay(4000); // 4초 동안 회전

    // 모터 정지
    digitalWrite(MOTOR_MT_P_PIN, LOW);
    digitalWrite(MOTOR_MT_N_PIN, LOW);
}

// DC 모터 회전 제어 멈춤 동작 반복 강력모드
void rotateMotor5(int direction) {
    for (int i = 0; i < 10; i++) { // 5번 반복
        if (direction == LEFT_ROTATE) {
            digitalWrite(MOTOR_MT_P_PIN, HIGH);
            digitalWrite(MOTOR_MT_N_PIN, LOW);
        }
        else if (direction == RIGHT_ROTATE) {
            digitalWrite(MOTOR_MT_P_PIN, LOW);
            digitalWrite(MOTOR_MT_N_PIN, HIGH);
        }

        delay(1000); // 1.5초 동안 회전

        // 모터 정지
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, LOW);

        delay(1000); // 0.5초 동안 정지
    }
}

// DC 모터 회전 제어 탈수모드
void rotateMotor6(int direction) {
    if (direction == LEFT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, HIGH);
        digitalWrite(MOTOR_MT_N_PIN, LOW);
    }
    else if (direction == RIGHT_ROTATE) {
        digitalWrite(MOTOR_MT_P_PIN, LOW);
        digitalWrite(MOTOR_MT_N_PIN, HIGH);
    }
    delay(20000); // 20초 동안 회전
    digitalWrite(MOTOR_MT_P_PIN, LOW); // 모터 정지
    digitalWrite(MOTOR_MT_N_PIN, LOW);
}

// 서브모터 동작 함수
void servoMoveToAngle(int angle) {
    int pulseWidth = (angle * 11) / 180 + 5; // 각도에 따른 펄스 폭 계산 (0도: 5, 180도: 25)
    softPwmWrite(SERVO_PIN, pulseWidth);
    delay(500); // 동작 시간 대기
}

// 1. Standard 선택 동작
void runStandardCycle() {
    const char* steps[] = {
       "1-Water In",
        "2-Washing",
        "3-Water Out",
        "4-Water In",
        "5-Rinse",
        "6-Water Out",
        "7-Dehydration"
    };

    // 1-Water In : 물 입력
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[0]); // "1-Water In" 출력
    waterinMotor(LEFT_ROTATE); // 물 입력
    delay(10000); // 10초 대기

    // 2-Washing : 모터 회전
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[1]); // "2-Washing" 출력
    rotateMotor2(LEFT_ROTATE); // 모터 회전
    delay(10000); // 10초 대기

    // 3-Water Out : 물 빠짐
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[2]); // "3-Water Out" 출력
    wateroutMotor(LEFT_ROTATE); // 물 출력
    delay(10000); // 10초 대기

    // 4-Water In: 물 입력
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[3]); // "4-Water In" 출력
    waterinMotor(LEFT_ROTATE); // 물 입력
    delay(10000); // 10초 대기

    // 5-Rinse: 헹굼 동작
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[4]); // "5-Rinse" 출력
    rotateMotor3(LEFT_ROTATE); // 모터 회전
    delay(10000); // 10초 대기

    // 6-Water Out: 물 배수
    lcdSendCommand(0x01); // 화면

    // 6-Water Out: 물 배수
    lcdSetCursor(0, 0);
    lcdPrint(steps[5]); // "6-Water Out" 출력
    wateroutMotor(LEFT_ROTATE); // 물 출력
    delay(10000); // 10초 대기

    // 7-Dehydration: 탈수 동작
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[6]); // "7-Dehydration" 출력
    rotateMotor(LEFT_ROTATE); // 모터 회전
    delay(10000); // 10초

    // BUZZER 울림
    softToneWrite(BUZZER_PIN, 1000); // 1kHz 톤 출력
    delay(2000); // 2초 동안 소리
    softToneWrite(BUZZER_PIN, 0); // 소리 끄기

    // 서보모터 회전
    servoMoveToAngle(180); // 서브모터 180도 회전
}

// 2. Strong 선택 동작
void runStrongCycle() {
    const char* steps[] = {
       "1-Water In",
        "2-Washing",
        "3-Water Out",
        "4-Water In",
        "5-Rinse",
        "6-Water Out",
        "7-Dehydration"
    };

    // 1-Water In : 물 입력
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[0]); // "1-Water In" 출력
    waterinMotor(LEFT_ROTATE); // 물 입력
    delay(10000); // 10초 대기

    // 2-Washing : 모터 회전
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[1]); // "2-Washing" 출력
    rotateMotor5(LEFT_ROTATE); // 모터 회전
    delay(20000); // 20초 대기

    // 3-Water Out : 물 빠짐
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[2]); // "3-Water Out" 출력
    wateroutMotor(LEFT_ROTATE); // 물 출력
    delay(10000); // 10초 대기

    // 4-Water In: 물 입력
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[3]); // "4-Water In" 출력
    waterinMotor(LEFT_ROTATE); // 물 입력
    delay(10000); // 10초 대기

    // 5-Rinse: 헹굼 동작
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[4]); // "5-Rinse" 출력
    rotateMotor3(LEFT_ROTATE); // 모터 회전
    delay(10000); // 10초 대기

    // 6-Water Out: 물 배수
    lcdSendCommand(0x01); // 화면

    // 6-Water Out: 물 배수
    lcdSetCursor(0, 0);
    lcdPrint(steps[5]); // "6-Water Out" 출력
    wateroutMotor(LEFT_ROTATE); // 물 출력
    delay(10000); // 10초 대기

    // 7-Dehydration: 탈수 동작
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[6]); // "7-Dehydration" 출력
    rotateMotor(LEFT_ROTATE); // 모터 회전
    delay(10000); // 10초

    // BUZZER 울림
    softToneWrite(BUZZER_PIN, 1000); // 1kHz 톤 출력
    delay(2000); // 2초 동안 소리
    softToneWrite(BUZZER_PIN, 0); // 소리 끄기

    // 서보모터 회전
    servoMoveToAngle(180); // 서브모터 180도 회전
}

// 3. Delicate 선택 동작
void runDelicateCycle() {
    const char* steps[] = {
       "1-Water In",
        "2-Washing",
        "3-Water Out",
        "4-Water In",
        "5-Rinse",
        "6-Water Out",
        "7-Dehydration"
    };

    // 1-Water In : 물 입력
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[0]); // "1-Water In" 출력
    waterinMotor(LEFT_ROTATE); // 물 입력
    delay(10000); // 10초 대기

    // 2-Washing : 모터 회전
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[1]); // "2-Washing" 출력
    rotateMotor2(LEFT_ROTATE); // 모터 회전
    delay(10000); // 10초 대기

    // 3-Water Out : 물 빠짐
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[2]); // "3-Water Out" 출력
    wateroutMotor(LEFT_ROTATE); // 물 출력
    delay(10000); // 10초 대기

    // 4-Water In: 물 입력
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[3]); // "4-Water In" 출력
    waterinMotor(LEFT_ROTATE); // 물 입력
    delay(10000); // 10초 대기

    // 5-Rinse: 헹굼 동작
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[4]); // "5-Rinse" 출력
    rotateMotor4(LEFT_ROTATE); // 모터 회전
    delay(20000); // 20초 대기

    // 6-Water Out: 물 배수
    lcdSendCommand(0x01); // 화면

    // 6-Water Out: 물 배수
    lcdSetCursor(0, 0);
    lcdPrint(steps[5]); // "6-Water Out" 출력
    wateroutMotor(LEFT_ROTATE); // 물 출력
    delay(10000); // 10초 대기

    // 7-Dehydration: 탈수 동작
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[6]); // "7-Dehydration" 출력
    rotateMotor(LEFT_ROTATE); // 모터 회전
    delay(10000); // 10초

    // BUZZER 울림
    softToneWrite(BUZZER_PIN, 1000); // 1kHz 톤 출력
    delay(2000); // 2초 동안 소리
    softToneWrite(BUZZER_PIN, 0); // 소리 끄기

    // 서보모터 회전
    servoMoveToAngle(180); // 서브모터 180도 회전
}

// 4. Dehydration 선택 동작
void runDehydrationCycle() {
    const char* steps[] = {
        "Dehydration"
    };

    // 1-Water In : 물 입력
    lcdSendCommand(0x01); // 화면 클리어
    delay(2);
    lcdSetCursor(0, 0);
    lcdPrint(steps[0]); // "Dehydration" 출력
    rotateMotor6(LEFT_ROTATE); // 모터 회전
    delay(20000); // 20초

    // BUZZER 울림
    softToneWrite(BUZZER_PIN, 1000); // 1kHz 톤 출력
    delay(2000); // 2초 동안 소리
    softToneWrite(BUZZER_PIN, 0); // 소리 끄기

    // 서보모터 회전
    servoMoveToAngle(180); // 서브모터 180도 회전
}

// 메뉴 탐색
void scrollMenu(int direction) {
    if (direction == 1) {
        menuIndex = (menuIndex + 1) % MENU_COUNT; // 아래로 스크롤
    }
    else if (direction == -1) {
        menuIndex = (menuIndex - 1 + MENU_COUNT) % MENU_COUNT; // 위로 스크롤
    }
    displayMenu();
}
// 메뉴 표시
void displayMenu() {
    lcdSendCommand(0x01); // 화면 클리어
    delay(2); // 클리어 후 지연

    lcdSetCursor(0, 0);
    lcdPrint(menuItems[menuIndex]);

    int nextIndex = (menuIndex + 1) % MENU_COUNT; // 다음 메뉴 인덱스
    lcdSetCursor(1, 0);
    char buffer[16];
    if (menuIndex == 0) {
        snprintf(buffer, 16, "Time: %d sec", 60); // Standard
    }
    else if (menuIndex == 1 || menuIndex == 2) {
        snprintf(buffer, 16, "Time: %d sec", 70); // Strong, Delicate
    }
    else if (menuIndex == 3) {
        snprintf(buffer, 16, "Time: %d sec", 20); // Dehydration
    }
    lcdPrint(buffer);
}

// 메뉴 선택
void selectMenu() {
    if (menuIndex >= 0 && menuIndex < MENU_COUNT) {
        lcdSendCommand(0x01);
        lcdSetCursor(0, 0);
        lcdPrint(menuItems[menuIndex]);

        servoMoveToAngle(0); // 문 닫기
        delay(2000); // 문 닫는 시간 대기

        float waterLevel = getWaterLevel();
        lcdSetCursor(1, 0);
        char buffer[16];
        snprintf(buffer, 16, "Water: %.2f cm", waterLevel);
        lcdPrint(buffer);
        delay(2000);

        if (waterLevel > 20.0) {

            if (menuIndex == 0) {
                servoMoveToAngle(0);  // 서브모터 원위치
                runStandardCycle();
            }
            else if (menuIndex == 1) {
                servoMoveToAngle(0);  // 서브모터 원위치
                runStrongCycle();
            }
            else if (menuIndex == 2) {
                servoMoveToAngle(0);  // 서브모터 원위치
                runDelicateCycle();
            }
            else if (menuIndex == 3) {
                servoMoveToAngle(0);  // 서브모터 원위치
                runDehydrationCycle();
            }

            lcdSendCommand(0x01);
            lcdSetCursor(0, 0);
            lcdPrint("Completed");
            servoMoveToAngle(180); // 문 열기

            displayMenu();
        }
        else {
            softToneWrite(BUZZER_PIN, 2000);
            delay(3000);
            softToneWrite(BUZZER_PIN, 0);

            lcdSendCommand(0x01);
            lcdSetCursor(0, 0);
            lcdPrint("Too low!");
            delay(2000);

            displayMenu();
        }
    }
}

// 메인 함수
int main() {
    wiringPiSetupGpio();
    lcdFd = wiringPiI2CSetup(LCD_ADDR);
    lcdInit();

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(SERVO_PIN, OUTPUT);
    softPwmCreate(SERVO_PIN, 0, 200);

    pinMode(WATER_IN_P_PIN, OUTPUT);
    pinMode(WATER_IN_N_PIN, OUTPUT);
    pinMode(WATER_OUT_P_PIN, OUTPUT);
    pinMode(WATER_OUT_N_PIN, OUTPUT);

    pinMode(MOTOR_MT_P_PIN, OUTPUT);
    pinMode(MOTOR_MT_N_PIN, OUTPUT);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    softToneCreate(BUZZER_PIN);

    keypadInit();
    displayMenu();

    while (1) {
        char key = getKey();
        if (key == 'A') {
            scrollMenu(1); // 아래로 스크롤
        }
        else if (key == 'B') {
            scrollMenu(-1); // 위로 스크롤
        }
        else if (key == 'C') {
            selectMenu(); // 메뉴 선택
        }
    }

    return 0;
}