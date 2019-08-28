#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length);

boolean checkWiFiConnection = false;

int counterDHT22 = 0;

char temperatureChar[10];
char humidityChar[10];

const char TEMPERATURE_PUB[40] = "home/device03/temperature";
const char HUMIDITY_PUB[40] = "home/device03/humidity";

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
  client.loop();
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
      if(counterDHT22 >= 240){
        TempAndHumidity measurement = dht.getTempAndHumidity(); 
        Serial.print("Temperature: ");
        Serial.println(measurement.temperature);
        Serial.print("Humidity: ");
        Serial.println(measurement.humidity);
        sprintf(temperatureChar, "%f", measurement.temperature);
        sprintf(humidityChar, "%f", measurement.humidity);
        client.publish(TEMPERATURE_PUB, temperatureChar);
        client.publish(HUMIDITY_PUB, humidityChar);
        counterDHT22=0;
      }
    }
    else{
      client.publish(TEMPERATURE_PUB, "-99");
      client.publish(HUMIDITY_PUB, "-99");
    }
    counterDHT22++;
    Serial.print("Get status string: ");
    Serial.println(dht.getStatusString());
    Serial.println(counterDHT22);
    delay(250);
  }
}
