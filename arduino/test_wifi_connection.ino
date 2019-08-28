#include <ESP8266WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int length);

float i=0;
char numberChar[10];
boolean checkWiFiConnection = false;

void setup() {
  delay(5000);
  //Serial Port begin
  Serial.begin(9600);
  WiFi.begin("SmartHomeAP", "smarthome");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, your IP address is: ");
  Serial.println(WiFi.localIP());
  
  client.setServer("192.168.1.162", 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("device02", "openhabian", "openhabian" )) {
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
  if(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    checkWiFiConnection = true;
  }
  else if(checkWiFiConnection==true && WiFi.status() == WL_CONNECTED){
    Serial.print("Connected, your IP address is: ");
    Serial.println(WiFi.localIP());
    client.setServer("192.168.1.162", 1883);
    client.setCallback(callback);
    while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
      if (client.connect("device02", "openhabian", "openhabian" )) {
        Serial.println("connected");  
      }
      else {
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000); 
      }
    }
    checkWiFiConnection = false;
  }
  else if(WiFi.status() == WL_CONNECTED){
    Serial.println(WiFi.status());
    sprintf(numberChar, "%f", i);
    client.publish("test", numberChar);
    i++;
    delay(2000);
  }
}
