#include <SoftwareSerial.h>

void setup(){
  //시리얼 시작
  Serial.begin(9600);
  Serial.println("AT 명령어를 쳐보세요:");
  //블루투스 시리얼 포트 시작
  Serial1.begin(9600); //bt_rx D18, bt_tx D19
}

void loop(){
  //hc06에서 모니터로 데이터 쓰기
  if (Serial1.available()){
    Serial.write(Serial1.read());
  }
  
  //시리얼 모니터네서 hc06으로 데이터 쓰기
  if (Serial.available()){
    Serial1.write(Serial.read());
  }  
}