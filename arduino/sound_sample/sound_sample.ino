const int sound_Pin = A0;
int sound;

void setup() {
  Serial.begin(115200);
}


int sound_read() {
  int sound = analogRead(sound_Pin);
  Serial.println("void sound");
  return sound;
}


void loop() {
  int sound = sound_read();
  Serial.println(sound);
  delay(200);
}
