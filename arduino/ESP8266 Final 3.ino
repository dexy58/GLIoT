#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length);

int relayLightRoom01 = 2;
int relayLightRoom04 = 16;

const char* ssid = "SmartHuawei";              //WiFi variables
const char* password =  "smartstudent";

const char* mqttServer = "192.168.43.24";    //MQTT variables
const int mqttPort = 1883;

const char* brokerUsername = "openhabian";
const char* brokerPassword = "openhabian";

const char SWITCH_SUB01[40]="home/room1/switchLight";
const char SWITCH_SUB04[40]="home/room4/switchLight";

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
 
    if (client.connect("device04", brokerUsername, brokerPassword )) {
      Serial.println("connected");  
    }
    else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000); 
    }
  }
  pinMode(relayLightRoom01, OUTPUT);
  pinMode(relayLightRoom04, OUTPUT);
  client.subscribe(SWITCH_SUB01);
  client.subscribe(SWITCH_SUB04);
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.println(topic);
  if (strcmp(topic,SWITCH_SUB01)==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayLightRoom01, HIGH);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayLightRoom01, LOW);
    }
  }
  if (strcmp(topic,SWITCH_SUB04)==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayLightRoom04, HIGH);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayLightRoom04, LOW);
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
    String clientId = "ESP8266Client04-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), brokerUsername, brokerPassword )) {
      Serial.println("Connected to broker");
      client.subscribe(SWITCH_SUB01);
      client.subscribe(SWITCH_SUB04);
    }
    else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000); 
    }
  }
  client.loop();
}
