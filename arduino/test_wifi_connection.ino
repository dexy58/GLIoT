#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int length);

float i=0;
char numberChar[10];

const char* ssid = "SmartHomeAP";              //WiFi variables
const char* password =  "smarthome";

const char* mqttServer = "192.168.1.162";    //MQTT variables
const int mqttPort = 1883;

const char* brokerUsername = "openhabian";
const char* brokerPassword = "openhabian";

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
      Serial.println("connected");  
    }
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000); 
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) { 
  
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
  if(WiFi.status() == WL_CONNECTED){
    Serial.println(WiFi.status());
    sprintf(numberChar, "%f", i);
    client.publish("test", numberChar);
    i++;
    delay(2000);
  }
}
