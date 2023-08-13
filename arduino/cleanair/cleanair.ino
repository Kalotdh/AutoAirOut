//상태
////모드 설정
int modeReal;
int modeShow;

const int modeReal_Pin = 51;
const int modeShow_Pin = 53;

////창문 상태
const int CLOSED = 0;
const int OPENED = 1;


//입력
////환기 버튼
const int CONTROL_PIN = 4;

////미세먼지
#define PMS_HEAD_1 0x42
#define PMS_HEAD_2 0x4d

int inDoorPms[4] = { 0, 0, 0, 0 };
int outDoorPms[4] = { 0, 0, 0, 0 };

////CO2
#include <MHZ19PWM.h>
const int MHZ19_PIN = 2;
MHZ19PWM mhz(MHZ19_PIN, MHZ_CONTINUOUS_MODE);

////소음
const int NOISE_PIN = A8;
int noise;

////라즈베리파이
const int PI_PIN = 11;


//출력
////창문 모터
#include <Servo.h>
Servo windowServo;
const int SERVO_PIN = 3;
int windowStatus = CLOSED;
const int CLOSED_WINDOW_ANGLE = 0;
const int OPENED_WINDOW_ANGLE = 90;

////환풍기
int FAN_IN_1 = 7;
int FAN_IN_2 = 6;
int fanStatus = CLOSED;

////LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);


//임계값
int IndoorLimit_Real = 50;
int IndoorLimit_Show = 50;

int OutdoorLimit_Real = 50;
int OutdoorLimit_Show = 50;

int CO2Limit_Real = 1500;
int CO2Limit_Show = 5000;

int NoiseLimit_Real = 1024;
int NoiseLimit_Show = 1024;

int IndoorLimit = IndoorLimit_Real;
int OutdoorLimit = OutdoorLimit_Real;
int CO2Limit = CO2Limit_Real;
int NoiseLimit = CO2Limit_Real;


void setup() {
  //메인 시리얼
  Serial.begin(9600);

  //모드
  pinMode(modeReal_Pin, INPUT);
  pinMode(modeShow_Pin, INPUT);
  pinMode(NOISE_PIN, INPUT);

  //환기 버튼
  pinMode(CONTROL_PIN, INPUT);

  //미세먼지
  Serial2.begin(9600);  //실내
  Serial3.begin(9600);  //실외

  //CO2
  mhz.useLimit(5000);

  //라즈베리 파이
  pinMode(PI_PIN, INPUT);

  //창문 모터
  windowServo.attach(SERVO_PIN);
  closeWindow();

  //환풍기
  pinMode(FAN_IN_1, OUTPUT);
  pinMode(FAN_IN_2, OUTPUT);

  //LCD
  lcd.init();
  lcd.backlight();
  
  Serial.println("--------start--------");
}

void loop() {
  //모드
  int modeReal = digitalRead(modeReal_Pin);
  int modeShow = digitalRead(modeShow_Pin);

  //미세먼지
  readIndoorPms();   //집안
  readOutDoorPms();  //집밖

  //CO2
  int CO2 = mhz.getCO2();
  //소음
  int noise = analogRead(NOISE_PIN);

  String co2noise = "CO2: " + String(CO2) + " NOE: "  + String(noise);
  Serial.println(co2noise);
  lcdrender(0, 2, co2noise);

  //환기 버튼
  int controlBtn = digitalRead(CONTROL_PIN);

  //라즈베리 파이
  int piBtn = digitalRead(PI_PIN);

  Serial.println("PI : " + String(piBtn));

  Serial.println("Mode : " + String(modeReal) + " : " + String(modeShow));

  //판단
  if (modeReal == 0 && modeShow == 0){
    Serial.println("nope");
    delay(200);

  } else {

    if(modeReal == 1) {
      IndoorLimit = IndoorLimit_Real;
      OutdoorLimit = OutdoorLimit_Real;
      CO2Limit = CO2Limit_Real;
      NoiseLimit = NoiseLimit_Real;

    } else if (modeShow == 1) {
      IndoorLimit = IndoorLimit_Show;
      OutdoorLimit = OutdoorLimit_Show;
      CO2Limit = CO2Limit_Show;
      NoiseLimit = NoiseLimit_Show;
    }

    //버튼 눌렀을 때 밖 미세먼지 측정 후 창문 열기
    if (controlBtn == 1) {
      Serial.println("버튼 on");
      if (readWindowStatus() == OPENED) {
        closeWindow();
      } else if (readWindowStatus() == CLOSED) {
        cleanAir();
      }
    } else {



      // 잠자는 학생이 있으면
      if (piBtn == 1) {  // 라즈베리 파이로 판단
        Serial.print("catch sleep");
        cleanAir();
      }
      
      //집안 미세먼지가 높으면 창을 열거나 환풍
      if (inDoorPms[3] > IndoorLimit) {  // 집안 미세먼지 높음
        Serial.print("IndoorLimit : " + String(inDoorPms[3]));
        cleanAir();
      }

      //co2가 높으면 높으면 창을 열거나 환풍
      if (CO2 > CO2Limit) {
        Serial.println("CO2Limit : " + String(CO2));
        cleanAir();
      }

      //밖의 미세먼지가 높으면 창문 닫기
      if (outDoorPms[3] > OutdoorLimit) {
        Serial.print("OutdoorLimit");
        closeWindow();
      }

      //시끄러우면 창닫기
      if (noise > NoiseLimit) {
        Serial.println("Noise Limit : " + String(noise));
        closeWindow();
      }

    }
  }
  delay(500);
}

void cleanAir() {
  if (outDoorPms[3] <= OutdoorLimit) {
    openWindow();
  } else {
    openFan(3000);
  }
}


//환기
////환풍기 작동
void openFan(int timeout) {
  lcdrender(0, 3, String("Fan is Running"));
  Serial.println("fan");
  digitalWrite(FAN_IN_1, HIGH);
  digitalWrite(FAN_IN_2, LOW);
  delay(timeout);
  digitalWrite(FAN_IN_1, LOW);
  digitalWrite(FAN_IN_2, LOW);
}


////창문 상태 읽기
int readWindowStatus() {
  int windowAngle = windowServo.read();
  Serial.println(windowAngle);
  if (windowAngle == OPENED_WINDOW_ANGLE) {
    windowStatus = OPENED;
  } else if (windowAngle == CLOSED_WINDOW_ANGLE) {
    windowStatus = CLOSED;
  } else {
    windowStatus = CLOSED;
    windowServo.write(CLOSED_WINDOW_ANGLE);
    delay(500);
  }
  return windowStatus;
}

////창문 열기
void openWindow() {
  if (readWindowStatus() == CLOSED) {
    lcdrender(0, 3, String("Open the Window"));
    windowServo.write(OPENED_WINDOW_ANGLE);
    delay(500);
  }
  Serial.println("Open the Window");
}

////창문 닫기
void closeWindow() {
  if (readWindowStatus() == OPENED) {
  lcdrender(0, 3, String("Close the Window"));
    windowServo.write(CLOSED_WINDOW_ANGLE);
    delay(500);
  }
  Serial.println("close the window");
}


//미세먼지
////실내 미세먼지
void readIndoorPms() {
  unsigned char pms[32];
  if (Serial2.available() >= 32) { 

    while (Serial2.read() != PMS_HEAD_1) {
      continue;
    }
    pms[0] = PMS_HEAD_1;

    for (int j = 1; j < 32; j++) {
      pms[j] = Serial2.read();
    }

    int PM1_0 = (pms[10] << 8) | pms[11];
    int PM2_5 = (pms[12] << 8) | pms[13];
    int PM10 = (pms[14] << 8) | pms[15];

    inDoorPms[0] = true;
    inDoorPms[1] = PM1_0;
    inDoorPms[2] = PM2_5;
    inDoorPms[3] = PM10;
  } else {
    inDoorPms[0] = false;
  }

  String inDoorPMS = String("I N: ") + String(inDoorPms[3]) + String(" ") + String(inDoorPms[2]) + String(" ") + String(inDoorPms[1]);
  lcdrender(0, 0, inDoorPMS);
  Serial.println(inDoorPMS);
}

////실외 미세먼지
void readOutDoorPms() {
  unsigned char pms2[32];
  if (Serial3.available() >= 32) {

    while (Serial3.read() != PMS_HEAD_1) {
      continue;
    }
    pms2[0] = PMS_HEAD_1;

    for (int j = 1; j < 32; j++) {
      pms2[j] = Serial3.read();
    }

    int PM1_0 = (pms2[10] << 8) | pms2[11];
    int PM2_5 = (pms2[12] << 8) | pms2[13];
    int PM10 = (pms2[14] << 8) | pms2[15];

    outDoorPms[0] = true;
    outDoorPms[1] = PM1_0;
    outDoorPms[2] = PM2_5;
    outDoorPms[3] = PM10;

  } else {
    outDoorPms[0] = false;
  }
  String outDoorPMS = String("OUT: ") + String(outDoorPms[3]) + String(" ") + String(outDoorPms[2]) + String(" ") + String(outDoorPms[1]);
  lcdrender(0, 1, outDoorPMS);
  Serial.println(outDoorPMS);
}

//LCD
void lcdrender(int x, int y, String text) {
  lcd.setCursor(x, y);
  lcd.print(text);
}