/* Complete code for GlobalLogic student project
 Ensure that you have all the libraries downloaded and working WiFi network
 Code publishes temperature  and humidity data from DHT22 sensor to the MQTT broker through Wifi network
 Code reads messages from MQTT topic "emergency/fan" and turns the fan on if needed
 Code publishes temperature from another temperature sensor DS180B20 to MQTT broker
 Code checks if correct 4-digit password has been enetered through keypad, if it has, published "login" to MQTT broker, else if there were 4 wrong inputs, 
 lockes the system and doesn't let you any more tries

 The circuit:
 - connect VCC sensor pin to 3.3V on ESP32
 - connect GND sensor pin to ESP32 GND pin
 - connect data pin od sensor to D5 pin on microcontroller
 - connect GND and VCC of sensor DS18B20 to 3.3V and GND of microcontroller
 - connect data pin of DS18B20 to 4,7 kohm resistor and then to D15
 - connect the motor and LED diode according to sketch from the pdf document (circuit with diode and npn transistor)
 - connect the keyboard as it is shown in picture in pdf file

 created by: Anita Garic, Edin Pjevic
 GlobalLogic student project
 
*/
//==========================================================================================================================================
//==========================================================================================================================================
#include <WiFi.h>          // Enables the ESP8266 to connect to the local network (via WiFi)
#include <PubSubClient.h>  // Allows us to connect to, and publish to the MQTT broker
#include <DHTesp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Keypad.h>
#define DHTpin 5
#define ONE_WIRE_BUS 15
#define motorPin 23
#define ledPin 22
#define relayLightRoom01_1 4
#define relayLightRoom01_2 16
#define relayLightRoom04_1 18
#define relayLightRoom04_2 19
//==========================================================================================================================================
//==========VARIABLES=======================================================================================================================
const char* ssid = "SmartHuawei";              //WiFi variables
const char* password =  "smartstudent";

const char* topics[] = {      //MQTT topics
  "home/room1/temp",
  "home/room1/hum",
  "home/room1/temp_ntc",
  "rasp/feedback",
  "emergency/fan",
  "home/room1/login",
  "home/room1/login/reset",
  "status/sensor",
  "home/room1/switchLight",
  "home/room4/switchLight",
  "home/room1/light",
  "home/room4/light"
};

const char* mqttServer = "192.168.43.24";    //MQTT variables (192.168.43.181 <== MyPi) (192.168.43.24 <== KingsPi)
const int mqttPort = 1883;
const char* brokerUsername = "openhabian";
const char* brokerPassword = "openhabian";

int raspReady = 1;                            //Sending control variables
int switcher = 5;

unsigned long previousMillis[3] = {0,0,0};            //Measurement frequency variables
unsigned long intervals[3] = {30000,30000,30000};

int counter[2] = {0,0};    //Measurement handling variable
 
WiFiClient espClient;       //Initialise the WiFi and MQTT Client objects
PubSubClient client(espClient);
DHTesp dht;
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

char temperatureChar[10];       //Measurement variables
char humidityChar[10];
char temperatureCharDS[10];
char charBuffMsg[80];

int fanWork = 0;

//Keyboard variables
const byte ROWS = 4; //Four rows
const byte COLS = 4; //Four columns
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27}; //Connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; //Connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char correctKeyboard[4] = {'6', '9', '6', '9'};
char passKeyboard[4] = {0,0,0,0};
int i = 0;
int count = 0;
int flag = 1;
int tryCount = 0;
char resetField[] = "reset";
//==========================================================================================================================================
//==========FUNCTIONS=======================================================================================================================
//Handling data publishing
void publishData(int index, String name, String data, const String topic){
  if(index!=-1){
    counter[index]=0;
  }
  char charBuff[80];
  char topicBuff[50];          
  String message =  String("{") + name + String(": ") + data + String(", topic: ") + topic + String("}");
  message.toCharArray(charBuff, 80);
  topic.toCharArray(topicBuff,50);
  client.publish(topicBuff, charBuff);
}
//Handling subscriptions
void callback(char* topic, byte* payload, unsigned int length) {
  if(strcmp(topic,topics[3])==0){             //Receiving Raspberry Pi status
    payload[length]= '\0';
    unsigned long value = atoi((char *)payload);
    if(value == 1){
      raspReady = value;
    }
    else{
      int index = value%10;
      intervals[index] = value-index;
    }
  }
  if(strcmp(topic,topics[6])==0){        //Reseting login sensor
     for (int i = 0; i < length; i++) {
           if( (char)payload[i] != resetField[i] ){
              break;
           }
           flag = 1;
           tryCount = 0;
           publishData(-1,"status","reseted",topics[5]);
     }
  }
  if(strcmp(topic,topics[4])==0){        //DC motor fan handling
    payload[length]= '\0';
    fanWork = atoi((char *)payload);
    if (fanWork == 1){
              digitalWrite(motorPin, HIGH);
              digitalWrite(ledPin, HIGH);
    } else {
              digitalWrite(motorPin, LOW);
              digitalWrite(ledPin, LOW);
    }
  }
  if (strcmp(topic,topics[8])==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayLightRoom01_1, HIGH);
      digitalWrite(relayLightRoom01_2, HIGH);
      char onRelay[4] ="ON";
      client.publish(topics[10], onRelay, true);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayLightRoom01_1, LOW);
      digitalWrite(relayLightRoom01_2, LOW);
      char offRelay[5] ="OFF";
      client.publish(topics[10], offRelay, true);
    }
  }
  if (strcmp(topic,topics[9])==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayLightRoom04_1, HIGH);
      digitalWrite(relayLightRoom04_2, HIGH);
      char onRelay[4] ="ON";
      client.publish(topics[11], onRelay, true);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayLightRoom04_1, LOW);
      digitalWrite(relayLightRoom04_2, LOW);
      char offRelay[5] ="OFF";
      client.publish(topics[11], offRelay, true);
    }
  }
 
  Serial.print("Message arrived in topic: ");       //Message Serial Print
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}
//Handling measurements
bool measuremntHandling(char *data) {
  if(strcmp(data,"nan")==0){
    counter[0]=counter[0]+1;
    if(counter[0]>=10){
      return false;
    }
    publishData(-1,"sensorStatus","DHT22Pending",topics[7]);
  }
  if(strcmp(data,"-127.00")==0){
    counter[1]=counter[1]+1;
    if(counter[1]>=5){
      return false;
    }
    publishData(-1,"sensorStatus","NTCPending",topics[7]);
  }
  return true;
}
//==========================================================================================================================================
//==========INITIAL SETUP===================================================================================================================
void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);                   //Connecting to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");      //Connecting to MQTT
    if(WiFi.status() == WL_CONNECTED){
      if (client.connect("ESP8266Client")) {
        Serial.println("connected");  
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
      }
    }
    delay(1000);
  }
  dht.setup(DHTpin, DHTesp::DHT22);               //Creating an object for temperature and humidity sensor
  sensors.begin(); 
  pinMode(motorPin, OUTPUT);
  pinMode (ledPin, OUTPUT);
  pinMode(relayLightRoom01_1, OUTPUT);
  pinMode(relayLightRoom01_2, OUTPUT);
  pinMode(relayLightRoom04_1, OUTPUT);
  pinMode(relayLightRoom04_2, OUTPUT);

  for(int i=0; i<(sizeof(topics)/sizeof(topics[0])); i++){    //Subscribing to all topics
    client.subscribe(topics[i]);
  }

  for(int i=0; i<(sizeof(previousMillis)/sizeof(previousMillis[0])); i++){    //Subscribing to all topics
    previousMillis[i] = millis();
  }
}
//==========================================================================================================================================
//==========THE LOOP========================================================================================================================
void loop() {
  /*while (!client.connected()) {
    Serial.println("Connecting to MQTT...");      //Reconnecting to MQTT
    if(WiFi.status() == WL_CONNECTED){
      if (client.connect("ESP8266Client")) {
        Serial.println("connected");  
      } else {
        Serial.print("failed with state ");
        Serial.print(client.state());
      }
    }
    delay(1000);
  }*/
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(WiFi.status());
  }
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), brokerUsername, brokerPassword )) {
      for(int i=0; i<(sizeof(topics)/sizeof(topics[0])); i++){    //Subscribing to all topics
      client.subscribe(topics[i]);
      }
    }
    else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000); 
    }
  }
  client.loop();

  char key = keypad.getKey();  //KeyPad Handling
  if(key){
     if(count<4 && flag != 2){
        passKeyboard[count]=key;
        for (i=0; i<4; i++){
          Serial.print(passKeyboard[i]);
          Serial.print("  ");
        }
        Serial.println("");
        count++; 
     }
     if (key == 'C' && flag != 2){
        for (i=0; i<4; i++){
            if(passKeyboard[i] != correctKeyboard[i]){
              flag = 0;
              break;
            } else {
              flag = 1;
            }
        }
        tryCount++;
        if(tryCount > 3){                   //after 4 tries keypad is locked, waiting for confirmation from raspberry
          flag = 2;
        }
        if(flag == 1){
          Serial.println("Authorized access!");
          tryCount = 0;
          publishData(-1,"status","login",topics[5]);
        }else if(flag == 2) {
          Serial.println("Too many tries! You have to wait for confirmation!");
          publishData(-1,"status","lockedout",topics[5]);
        } else {
          Serial.println("Unauthorized access!");
        }
        count = 0;
        for (i=0; i<4; i++){
          passKeyboard[i]= 0;
        }
     }
  }

 unsigned long currentMillis = millis();

 if(raspReady==1){        //Measurement publishing
   switch(switcher){
     case 5: {
       if(currentMillis-previousMillis[0]>intervals[0] || currentMillis-previousMillis[0]<0){
         Serial.println(intervals[0]);
         TempAndHumidity measurement = dht.getTempAndHumidity();
         dtostrf(measurement.temperature,5,2,temperatureChar);
         bool status = measuremntHandling(temperatureChar);
         if(status && strcmp(temperatureChar,"nan")!=0){
           publishData(0,"temp",temperatureChar,topics[0]);
           publishData(-1,"sensorStatus","DHT22Active",topics[7]);
         }
         else if(!status){
           publishData(-1,"sensorStatus","DHT22NotResponding",topics[7]);
         }
         raspReady=0;
         previousMillis[0] = currentMillis;
       }
       switcher++;
       break;
     }
     case 6: {
       if(currentMillis-previousMillis[1]>intervals[1] || currentMillis-previousMillis[1]<0){
         Serial.println(intervals[1]);
         TempAndHumidity measurement = dht.getTempAndHumidity();
         dtostrf(measurement.humidity,5,2,humidityChar);
         bool status = measuremntHandling(humidityChar);
         if(status && strcmp(humidityChar,"nan")!=0){
           publishData(0,"hum",humidityChar,topics[1]);
           publishData(-1,"sensorStatus","DHT22Active",topics[7]);
         }
         else if(!status){
           publishData(-1,"sensorStatus","DHT22NotResponding",topics[7]);
         }
         raspReady=0;
         previousMillis[1] = currentMillis;
       }
       switcher++;
       break;
     }
     case 7: {
       if(currentMillis-previousMillis[2]>intervals[2] || currentMillis-previousMillis[2]<0){
         Serial.println(intervals[2]);
         sensors.requestTemperatures(); 
         dtostrf(sensors.getTempCByIndex(0),5,2,temperatureCharDS);
         bool status = measuremntHandling(temperatureCharDS);
         if(status && strcmp(temperatureCharDS,"-127.00")!=0){
           publishData(1,"temp",temperatureCharDS,topics[2]);
           publishData(-1,"sensorStatus","NTCActive",topics[7]);
         }
         else if(!status){
           publishData(-1,"sensorStatus","NTCNotResponding",topics[7]);
         }
         raspReady=0;
         previousMillis[2] = currentMillis;
       }
       switcher=5;
       break;
     }
   }
 }
}
