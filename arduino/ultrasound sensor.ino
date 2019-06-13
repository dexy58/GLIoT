#include <Servo.h> // including servo library.

Servo servo_1; // Giving name to servo.

int trigPin = 15;    // Trigger
int echoPin = 2;    // Echo
long duration, cm, inches;

long distanceArray[180];
boolean flag = true;
int counter=0;
 
void setup() {
  //Serial Port begin
  delay(10000);
  Serial.begin (9600);
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  servo_1.attach(16); // Attaching Servo to 16
}
 
void loop() {
  int returnBack = 180;
  if(flag == true){
    for(int i=0;i<180;i++){
      servo_1.write(i);
      digitalWrite(trigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      pinMode(echoPin, INPUT);
      duration = pulseIn(echoPin, HIGH);
      cm = (duration/2) / 29.1;
      distanceArray[i] = cm;
      delay(20);
    }
    flag = false;
    delay(1000);
  }
  else{
    servo_1.write(0);
    delay(250);
    for(int i=0;i<180;i++){
      servo_1.write(i);
      digitalWrite(trigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      pinMode(echoPin, INPUT);
      duration = pulseIn(echoPin, HIGH);
      cm = (duration/2) / 29.1;
      Serial.print("Izmjereno sada: ");
      Serial.println(cm);
      Serial.print("Izmjereno prije: ");
      Serial.println(distanceArray[i]);
      if(abs(cm-distanceArray[i])>distanceArray[i]*0.4 && counter>=0 && counter<=1){
        Serial.println("Potencijalni provalnik");
        counter++;
      }
      else if(abs(cm-distanceArray[i])>distanceArray[i]*0.4 && counter>1){
        Serial.println("Provalnik");
        counter=0;
        flag = true;
        returnBack = i;
        break;
      }
      else{
        distanceArray[i]=cm;
        counter=0;
      }     
      delay(150);
    }
  }
  for(int returnNow = returnBack; returnNow>=1; returnNow-=1){                                
    servo_1.write(returnNow);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
}
