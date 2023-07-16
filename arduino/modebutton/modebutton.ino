const int modeRead_Pin = 51;
const int modeShow_Pin = 53;

int modeReal;
int modeShow;

void setup() {
    Serial.begin(115200);
    Serial.println("starting");
    pinMode(modeRead_Pin, INPUT);
    pinMode(modeShow_Pin, INPUT);
}

void loop() {
    int modeReal = digitalRead(modeRead_Pin);
    int modeShow = digitalRead(modeShow_Pin);

    Serial.print("Real:");
    Serial.print(modeReal);
    Serial.print("Show:");
    Serial.println(modeShow);
}
