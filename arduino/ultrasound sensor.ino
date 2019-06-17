#include <Servo.h> // including servo library.

Servo servo_1; // Giving name to servo.

int trigPin = 15;    // Trigger
int echoPin = 14;    // Echo
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
  int returnBack = 0;
  if(flag == true){
    counter = 0;
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
      delay(50);
    }
    returnBack = 179;
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
      if(abs(cm-distanceArray[i])>distanceArray[i]*0.4 && counter>=0 && counter<=3){
        Serial.println("Potencijalni provalnik");
        counter++;
      }
      else if(abs(cm-distanceArray[i])>distanceArray[i]*0.4 && counter>3){
        Serial.println("Provalnik");
        counter=0;
        flag = true;
        returnBack = i;
        break;
      }
      else{
        distanceArray[i]=(distanceArray[i]+cm)/2.0;
        counter=0;
      }     
      delay(50);
    }
    if(flag == false){
      for(int i=179;i>=0;i--){
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
        if(abs(cm-distanceArray[i])>distanceArray[i]*0.4 && counter>=0 && counter<=3){
          Serial.println("Potencijalni provalnik");
          counter++;
        }
        else if(abs(cm-distanceArray[i])>distanceArray[i]*0.4 && counter>3){
          Serial.println("Provalnik");
          counter=0;
          flag = true;
          returnBack = i;
          break;
        }
        else{
          distanceArray[i]=(distanceArray[i]+cm)/2.0;
          counter=0;
        }     
        delay(50);      
      }   
    }
  }
  for(int returnNow = returnBack; returnNow>=0; returnNow--){                                
    servo_1.write(returnNow);
    delay(10);
  } 
}
