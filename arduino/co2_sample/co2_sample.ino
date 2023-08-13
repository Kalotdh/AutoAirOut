#include <MHZ19PWM.h>
#include <C302_Steam_Air_MH-Z19B_CO2.h>

const int MHZ19_Pin = 2;
MHZ19PWM mhz(MHZ19_Pin, MHZ_CONTINUOUS_MODE);
int CO2;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Starting..."));
}

void loop() {
  readCO2();
  Serial.println(CO2);
  delay(1000);
}

void readCO2() {
  int CO2 = mhz.getCO2();
  Serial.println(CO2);
}