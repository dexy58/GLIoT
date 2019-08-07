int trigPin = 15;    // Trigger
int echoPin = 14;    // Echo
long duration, cm, inches;
long lastDistance;
boolean flagDistance = false;
int counterDistance = 0;
 
void setup() {
  //Serial Port begin
  Serial.begin (9600);
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}
 
void loop() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
 
  // Convert the time into a distance
  cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  if(flagDistance == false){
    lastDistance = cm;
    flagDistance=true;
  }
  else if(flagDistance == true){
    if(abs(cm-lastDistance)>lastDistance*0.4 && counterDistance>=0 && counterDistance<=3){
      Serial.println("Potencijalni provalnik");
      counterDistance++;
    }
    else if(abs(cm-lastDistance)>lastDistance*0.4 && counterDistance>=4){
      Serial.println("Provalnik");
      counterDistance=0;
      flagDistance=false;
    }
    else{
      lastDistance = (lastDistance+cm)/2.0;
      counterDistance=0;
    }
  }
  
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();
  
  delay(250);
}
