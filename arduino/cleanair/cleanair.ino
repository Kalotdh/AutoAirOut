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
////버튼
const int Control_Pin = 4;

////미세먼지
#define PMS_HEAD_1 0x42
#define PMS_HEAD_2 0x4d

int inDoorPms[4] = { 0, 0, 0, 0 };
int outDoorPms[4] = { 0, 0, 0, 0 };

String inDoorPMS;
String outDoorPMS;

////이산화탄소
#include <MHZ19PWM.h>
const int MHZ19_Pin = 2;
MHZ19PWM mhz(MHZ19_Pin, MHZ_CONTINUOUS_MODE);

////소음
const int noise_Pin = A0;
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
LiquidCrystal_I2C lcd(0x27, 16, 2);


//임계값
int IndoorLimit;
int OutdoorLimit;
int CO2Limit;

int IndoorLimit_Real = 75;
int OutdoorLimit_Real = 82;

int IndoorLimit_Show = 50;
int OutdoorLimit_Show = 50;

int CO2Limit_Real = 1000;
int CO2Limit_Show = 500;



void setup() {
  //메인 시리얼
  Serial.begin(115200);

  //모드
  pinMode(modeReal_Pin, INPUT);
  pinMode(modeShow_Pin, INPUT);

  //버튼
  pinMode(Control_Pin, INPUT);

  //PMS5003 센서 시리얼
  Serial2.begin(9600);  //실내 PMS5003|tx2
  Serial3.begin(9600);  //실외 PMS5003|tx3

  //CO2
  mhz.useLimit(5000);
  closeWindow();

  //라즈베리 파이
  pinMode(PI_PIN, INPUT);

  //창문 모터
  windowServo.attach(SERVO_PIN);

  //환풍기
  pinMode(FAN_IN_1, OUTPUT);
  pinMode(FAN_IN_2, OUTPUT);

  //LCD
  lcd.init();
  lcd.backlight();
  
  Serial.println("--------start--------");
}

void loop() {

  //미세먼지 농도 읽기
  readIndoorPms();   //집안
  readOutDoorPms();  //집밖
  
  //CO2 농도 읽기
  int CO2 = mhz.getCO2();

  int noise = analogRead(noise_Pin);

  //환기버튼 인식
  int Control = digitalRead(Control_Pin);

  //잠자는 사람 감지
  int pi_bt = digitalRead(PI_PIN);

  int modeReal = digitalRead(modeReal_Pin);
  int modeShow = digitalRead(modeShow_Pin);

/*
  Serial.print("Real:");
  Serial.print(modeReal);
  Serial.print("Show:");
  Serial.println(modeShow);
*/

  if(modeReal == 0 && modeShow == 0){

  } else {

    if(modeReal == 1) {
      IndoorLimit = IndoorLimit_Real;
      OutdoorLimit = OutdoorLimit_Real;
      CO2Limit = CO2Limit_Real;
    } else if(modeShow == 1) {
      IndoorLimit = IndoorLimit_Show;
      OutdoorLimit = OutdoorLimit_Show;
      CO2Limit = CO2Limit_Show;
    }


    //버튼 눌렀을 때 밖 미세먼지 측정 후 창문 열기
    if (Control == 1) {
      Serial.println("버튼 on");
      //창문 열렸는지 확인하고 열리면 닫고
      //닫히면 외부 미먼 측정하고 ㄱㅊ으면 열고
      if (readWindowStatus() == OPENED) {
        closeWindow();

      } else if (readWindowStatus() == CLOSED) {
        cleanAir();
      }

    } else {
      if (outDoorPms[3] > 50) {
        Serial.print("집밖의 미세먼지가 높아서 창을 닫음");
        Serial.println(outDoorPms[3]);
        closeWindow();
      }

      if (pi_bt == 1) {  // 라즈베리 파이로 판단
        Serial.print("잠자서 환기 ");
        cleanAir();
      }

      if (inDoorPms[3] > 80) {  // 집안 미세먼지 높음
        Serial.print("집안의 미세먼지가 높아서 환기 ");
        Serial.println(inDoorPms[3]);
        cleanAir();
      }


      if (CO2 > 1000) {
        Serial.println("co2 높음");
        cleanAir();
      }

      if (noise > 1000) {
        Serial.println("소음 심함");
        closeWindow();
      }

    delay(100);
    }
  }
}

void cleanAir() {
  if (outDoorPms[3] <= 50) {
    openWindow();
  } else {
    openFan(3000);
  }
}



void openFan(int timeout) {
  lcdrender(1, String("Fan is Running"));
  if (fanStatus == CLOSED) {
    digitalWrite(FAN_IN_1, HIGH);
    digitalWrite(FAN_IN_2, LOW);
    fanStatus = OPENED;
    delay(timeout);
    closeFan();
  }
}

void closeFan() {
  if (readFanStatus() == OPENED) {
    digitalWrite(FAN_IN_1, LOW);
    digitalWrite(FAN_IN_2, LOW);
    fanStatus = CLOSED;
  }
}

/*
 창을 열고 닫기
*/


int readWindowStatus() {
  if (windowServo.read() == OPENED_WINDOW_ANGLE) {
    windowStatus = OPENED;
  } else if (windowServo.read() == CLOSED_WINDOW_ANGLE) {
    windowStatus = CLOSED;
  }
  return windowStatus;
}

void openWindow() {
  if (readWindowStatus() == CLOSED) {
    lcdrender(1, String("Open the Window"));
    windowServo.write(OPENED_WINDOW_ANGLE);
    delay(500);
  }
}

void closeWindow() {
  if (readWindowStatus() == OPENED) {
  lcdrender(1, String("Close the Window"));
    windowServo.write(CLOSED_WINDOW_ANGLE);
    delay(500);
  }
  Serial.println("come");
}


/*
 미세먼지 측정하기
*/
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
  lcdrender(0, inDoorPMS);
}

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
  lcdrender(1, outDoorPMS);
}

void lcdrender(int y, String text) {
  lcd.setCursor(0, y);
  lcd.print("                ");
  lcd.setCursor(0, y);
  lcd.print(text);
}