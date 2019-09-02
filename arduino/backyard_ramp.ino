#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

WiFiClient espClient;
PubSubClient client(espClient);
Servo servo_ramp;

int servo_pin = 16;
int servo_ramp_lowered = 90;
int servo_ramp_raised = 179;

const char* ssid = "SmartHomeAP";              //WiFi variables
const char* password =  "smarthome";

const char* mqttServer = "192.168.1.162";    //MQTT variables
const int mqttPort = 1883;

const char* brokerUsername = "openhabian";
const char* brokerPassword = "openhabian";

const char BACKYARD_RAMP_PUB[40] = "home/device03/ramp";

void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  delay(5000);
  //Serial Port begin
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, your IP address is: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("device02", brokerUsername, brokerPassword )) {
      Serial.println("Connected to broker");  
    }
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000); 
    }
  }
  servo_ramp.attach(servo_pin);
  servo_ramp.write(servo_ramp_lowered);
  client.subscribe(BACKYARD_RAMP_PUB);
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic,BACKYARD_RAMP_PUB)==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.println("Raise ramp");
      for(int i = servo_ramp_lowered; i<=servo_ramp_raised;i++){
        servo_ramp.write(i);
        delay(15);
      }
    }
    else if (!strncmp((char *)payload, "OFF", length)) {
      Serial.println("Lower ramp");
      for(int i = servo_ramp_raised; i>=servo_ramp_lowered;i--){
        servo_ramp.write(i);
        delay(15);
      }
    }
  }
}

void loop() {
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(WiFi.status());
  }
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");
 
    if (client.connect("device02", brokerUsername, brokerPassword )) {
      Serial.println("Connected to broker");  
    }
    else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000); 
    }
  }
  client.loop();
}
