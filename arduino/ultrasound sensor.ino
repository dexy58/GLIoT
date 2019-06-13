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
}
 
void loop() {
  if(flag == true){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);
    cm = (duration/2) / 29.1;
    for(int i=0;i<180;i++){
      distanceArray[i] = cm;
    }
    flag = false;
    delay(1000);
  }
  else{
    for(int i=0;i<180;i++){
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
      if(abs(cm-distanceArray[i])>distanceArray[i]*0.3 && counter==0){
        Serial.println("Potencijalni provalnik");
        counter++;
      }
      else if(abs(cm-distanceArray[i])>distanceArray[i]*0.3 && counter>1){
        Serial.println("Provalnik");
        counter=0;
      }
      else{
        distanceArray[i]=cm;
        counter=0;
      }     
      delay(250);
    }
    delay(1000);
  }
}
