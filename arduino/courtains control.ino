#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int length);

int STBY = 16; //standby

//Motor A
int PWMA = 14; //Speed control
int AIN1 = 12; //Direction
int AIN2 = 13; //Direction

boolean flag = true;

void setup(){
   delay(5000);
   Serial.begin(9600);
   while (!Serial);
   WiFi.begin("SmartHomeAP", "smarthome");
   Serial.print("Connecting");
   while (WiFi.status() != WL_CONNECTED)
   {
     delay(500);
     Serial.print(".");
   }
   Serial.println();
   Serial.print("Connected, IP address: ");
   Serial.println(WiFi.localIP());
   client.setServer("192.168.1.162", 1883);
   client.setCallback(callback);
   while (!client.connected()) {
     Serial.println("Connecting to MQTT...");
     if(client.connect("device01", "openhabian", "openhabian" )) {
       Serial.println("Connected");  
     }else {
       Serial.print("Failed with state ");
       Serial.print(client.state());
       delay(2000); 
     }
   }
   client.subscribe("home/device01/switchCurtains");
   pinMode(STBY, OUTPUT);
   pinMode(PWMA, OUTPUT);
   pinMode(AIN1, OUTPUT);
   pinMode(AIN2, OUTPUT);
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  if (strcmp(topic,"home/device01/switchCurtains")==0){
    if (!strncmp((char *)payload, "ON", length) && flag==false) {
      Serial.print("ON");
      move(1,200,0);
      flag=true;
      delay(2500);
      stop();
    }
    else if(!strncmp((char *)payload, "OFF", length) && flag==true){
      Serial.print("OFF");
      move(1,200,1);
      flag = false;
      delay(2500);
      stop();
    }
  }
}

void loop(){
  client.loop();
}

void move(int motor, int speed, int direction){
   //Move specific motor at speed and direction
   //motor: 0 for B, 1 for A
   //speed: 0 is off, and 255 is full speed
   //direction: 0 clockwise, 1 counter-clockwise
   digitalWrite(STBY, HIGH); //disable standby
   boolean inPin1 = LOW;
   boolean inPin2 = HIGH;
   if(direction == 1){
   inPin1 = HIGH;
   inPin2 = LOW;
   }
   if(motor == 1){
      digitalWrite(AIN1, inPin1);
      digitalWrite(AIN2, inPin2);
      analogWrite(PWMA, speed);
   }
}

void stop(){
   //enable standby
   digitalWrite(STBY, LOW);
}
