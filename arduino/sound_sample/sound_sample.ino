const int sound_Pin = A0;
int sound;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int sound = analogRead(sound_Pin);
  Serial.println(sound);
  delay(200);
}
