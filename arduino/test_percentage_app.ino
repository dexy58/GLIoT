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
const char FAN_MAX_TEMP_SUB[40]="home/device02/tempMaxFan";
const char FAN_MIN_TEMP_SUB[40]="home/device02/tempMinFan";
const char HEATING_MAX_TEMP_SUB[40]="home/device02/tempMaxHeat";
const char HEATING_MIN_TEMP_SUB[40]="home/device02/tempMinHeat";
const int FAN_MAX_TEMP = 26;
const int FAN_MIN_TEMP = 23;
const int HEAT_MAX_TEMP = 11;
const int HEAT_MIN_TEMP = 0;

float fanMaxTemp = FAN_MAX_TEMP;
float fanMinTemp = FAN_MIN_TEMP;
float result;
float percentage;
float heatMaxTemp = HEAT_MAX_TEMP;
float heatMinTemp= HEAT_MIN_TEMP;
float heat75percent=8.25;
float heat50percent=5.5;
float heat25percent=2.75;

int heating_body = 16;

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
      Serial.println("Connected to broker");  
    }
    else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000); 
    }
  }
  dht.setup(2, DHTesp::DHT22);
  client.subscribe(FAN_MAX_TEMP_SUB);
  client.subscribe(FAN_MIN_TEMP_SUB);
  client.subscribe(HEATING_MAX_TEMP_SUB);
  client.subscribe(HEATING_MIN_TEMP_SUB);
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic,FAN_MAX_TEMP_SUB)==0){
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    result = atoi(payload_string);
    percentage = (float)result/100.0;
    fanMaxTemp = FAN_MAX_TEMP+2*percentage;
    Serial.print("Current max temp: ");
    Serial.println(fanMaxTemp);
  }
  else if(strcmp(topic,FAN_MIN_TEMP_SUB)==0){
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    result = atoi(payload_string);
    percentage = (float)result/100.0;
    fanMinTemp = FAN_MIN_TEMP+2*percentage;
    Serial.print("Current min temp: ");
    Serial.println(fanMinTemp);
  }
  else if (strcmp(topic,HEATING_MAX_TEMP_SUB)==0){
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    result = atoi(payload_string);
    percentage = (float)result/100.0;
    heatMaxTemp = HEAT_MAX_TEMP+10*percentage;
    Serial.print("Current max temp: ");
    Serial.println(heatMaxTemp);
    result = (heatMaxTemp - heatMinTemp)/4;
    heat25percent = heatMinTemp + result;
    heat50percent = heatMinTemp + 2*result;
    heat75percent = heatMinTemp + 3*result;
    Serial.println(heat25percent);
    Serial.println(heat50percent);
    Serial.println(heat75percent);
  }
  else if (strcmp(topic,HEATING_MIN_TEMP_SUB)==0){
    char payload_string[length + 1];
    memcpy(payload_string, payload, length);
    payload_string[length] = '\0';
    result = atoi(payload_string);
    percentage = (float)result/100.0;
    heatMinTemp = HEAT_MIN_TEMP+10*percentage;
    Serial.print("Current min temp: ");
    Serial.println(heatMinTemp);
    result = (heatMaxTemp - heatMinTemp)/4;
    heat25percent = heatMinTemp + result;
    heat50percent = heatMinTemp + 2*result;
    heat75percent = heatMinTemp + 3*result;
    Serial.println(heat25percent);
    Serial.println(heat50percent);
    Serial.println(heat75percent);
  }
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
        if(measurement.temperature<=heatMinTemp){
          analogWrite(heating_body, 1024); //100% power
        }
        else if(measurement.temperature>heatMinTemp && measurement.temperature<=heat25percent){
          analogWrite(heating_body, 819); //80% power
        }
        else if(measurement.temperature>heat25percent && measurement.temperature<=heat50percent){
          analogWrite(heating_body, 614); //60& power
        }
        else if(measurement.temperature>heat50percent && measurement.temperature<=heat75percent){
          analogWrite(heating_body, 409); //40% power
        }
        else if(measurement.temperature>heat75percent && measurement.temperature<=heatMaxTemp){
          analogWrite(heating_body, 204); //20% power
        }
        else if(measurement.temperature>heatMaxTemp){
          analogWrite(heating_body, 0); //0% power
        }
      }
    }
    else{
      client.publish(TEMPERATURE_PUB, "-99");
      client.publish(HUMIDITY_PUB, "-99");
    }
    counterDHT22++;
    //Serial.print("Get status string: ");
    //Serial.println(dht.getStatusString());
    //Serial.println(counterDHT22);
    delay(250);
  }
}
