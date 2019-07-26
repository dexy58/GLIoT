int valueSensor1;

void setup() {
  pinMode(17, INPUT);
  Serial.begin(9600);

}

void loop() {
  valueSensor1 = analogRead(17);
  Serial.println(valueSensor1);
  delay(500);
}
