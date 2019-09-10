#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length);

const char* ssid = "SmartHuawei";              //WiFi variables
const char* password =  "smartstudent";

const char* mqttServer = "192.168.43.24";    //MQTT variables
const int mqttPort = 1883;

const char* brokerUsername = "openhabian";
const char* brokerPassword = "openhabian";

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

int heating_body = 16;
int dht22Pin = 2;

//equation for a line y=ax+b
//a=(y2-y1)/(x2-x1)
//b=y1-[(y2-y1)/(x2-x1)]*x1
//x2=heatMaxTemp
//x1=heatMinTemp
float lineAValue=0;
float lineBValue=0;
int y2Value=1024;
int y1Value=0;

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
  dht.setup(dht22Pin, DHTesp::DHT22);
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
    lineAValue = (y2Value-y1Value)/(heatMaxTemp-heatMinTemp);
    lineBValue = y1Value - heatMinTemp*((y2Value-y1Value)/(heatMaxTemp-heatMinTemp));
    Serial.print("Current line: y=");
    Serial.print(lineAValue);
    Serial.print("x");
    if(lineBValue>=0){
      Serial.print("+");
    }
    Serial.println(lineBValue);
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
    lineAValue = (y2Value-y1Value)/(heatMaxTemp-heatMinTemp);
    lineBValue = y1Value - heatMinTemp*((y2Value-y1Value)/(heatMaxTemp-heatMinTemp));
    Serial.print("Current line: y=");
    Serial.print(lineAValue);
    Serial.print("x");
    if(lineBValue>=0){
      Serial.print("+");
    }
    Serial.println(lineBValue);
  }
}

void loop() {
  client.loop();
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
        if(measurement.temperature<=heatMaxTemp && measurement.temperature>=heatMinTemp){
          //turn on heater
          int getPWM = lineAValue*measurement.temperature+lineBValue;
          Serial.print("Current PWM value: ");
          Serial.println(getPWM);
          if(getPWM<0){
            analogWrite(heating_body, 0);
          }
          else if(getPWM>1024){
            analogWrite(heating_body, 1024);
          }
          else{
            analogWrite(heating_body, getPWM);
          }
        }
        else{
          //turn off heater
          analogWrite(heating_body, 0);
        }
      }
    }
    else{
      client.publish(TEMPERATURE_PUB, "-99");
      client.publish(HUMIDITY_PUB, "-99");
    }
    counterDHT22++;
    delay(250);
  }
}
