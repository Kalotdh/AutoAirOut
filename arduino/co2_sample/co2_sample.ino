#include <MHZ19PWM.h>

const int MHZ19_Pin = 2;
MHZ19PWM mhz(MHZ19_Pin, MHZ_CONTINUOUS_MODE);

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting..."));

  delay(2000);

  mhz.useLimit(5000);
}

void loop() {
  int co2 = readCO2();
  Serial.println(co2);
  delay(1000);
}

float readCO2() {
  float co2 = mhz.getCO2();
  Serial.println(co2);
  return co2;
}