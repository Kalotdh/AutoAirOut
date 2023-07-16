#include <SoftwareSerial.h>

const int BT_TX = 2;
const int BT_RX = 3;

SoftwareSerial bluetooth(BT_TX, BT_RX);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  bluetooth.begin(9600);

}

void loop() {
  //hc06에서 모니터로 데이터 쓰기
  if (bluetooth.available()){
    Serial.write(bluetooth.read());
  }
  
  //시리얼 모니터네서 hc06으로 데이터 쓰기
  if (Serial.available()){
    bluetooth.write(Serial.read());
  }  
}
