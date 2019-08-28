#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length);

boolean checkWiFiConnection = false;

char temperatureChar[10];
char humidityChar[10];

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
  dht.setup(2, DHTesp::DHT22);
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
    if(strcmp(dht.getStatusString(),"OK")==0){
      TempAndHumidity measurement = dht.getTempAndHumidity(); 
      Serial.print("Temperature: ");
      Serial.println(measurement.temperature);
      Serial.print("Humidity: ");
      Serial.println(measurement.humidity);
      sprintf(temperatureChar, "%f", measurement.temperature);
      sprintf(humidityChar, "%f", measurement.humidity);
      client.publish("home/device03/temperature", temperatureChar);
      client.publish("home/device03/humidity", humidityChar);
    }
    else{
      client.publish("home/device03/temperature", "-99");
      client.publish("home/device03/humidity", "-99");
    }
    Serial.print("Get status string");
    Serial.println(dht.getStatusString());
    delay(2000);
  }
}
