//A sketch to demonstrate the tone() function

//Specify digital pin on the Arduino that the positive lead of piezo buzzer is attached.
int piezoPin = 2;

void setup() {
}

void loop() {
   tone(piezoPin, 1000, 500);
}
