#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length);

int trigPin = 15;    // Trigger
int echoPin = 14;    // Echo
long duration, inches;
float cm, lastDistance;
boolean flagDistance = false;
int counterDistance = 0;
boolean alarmFlag = true;

// MPU6050 Slave Device Address
const uint8_t MPU6050SlaveAddress = 0x68;
// Select SDA and SCL pins for I2C communication 
const uint8_t scl = 5;
const uint8_t sda = 4;

// sensitivity scale factor respective to full scale setting provided in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131;

// MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;

int relayPinBulb = 12;
int relayPinFan = 13;
int buzzerPin = 2;
int heating_body = 16;

int counterTemperature = 0;
char temperatureChar[50];
char distanceChar[50];

const char* ssid = "SmartHuawei";              //WiFi variables
const char* password =  "smartstudent";

const char* mqttServer = "192.168.43.24";    //MQTT variables
const int mqttPort = 1883;

const char* brokerUsername = "openhabian";
const char* brokerPassword = "openhabian";

const char TEMPERATURE_PUB[40] = "home/room2/temp";
const char FAN_MAX_TEMP_SUB[40]="home/room2/tempMaxFan";
const char FAN_MIN_TEMP_SUB[40]="home/room2/tempMinFan";
const char HEATING_MAX_TEMP_SUB[40]="home/room2/tempMaxHeat";
const char HEATING_MIN_TEMP_SUB[40]="home/room2/tempMinHeat";
const char ALARM_SUB[40] = "home/room2/alarmSwitch";
const char ALARM_PUB[40] = "home/room2/alarm";
const char SWITCH_SUB[40]="home/room2/switchLight";
const char SWITCH_PUB[40]="home/room2/light";
const char DISTANCE_PUB[40]="home/room2/dist";
const char RASP_FEEDBACK[40]="rasp/feedback02";
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

//equation for a line y=ax+b
//a=(y2-y1)/(x2-x1)
//b=y1-[(y2-y1)/(x2-x1)]*x1
//x2=heatMaxTemp
//x1=heatMinTemp
float lineAValue=0;
float lineBValue=0;
int y1Value=1024;
int y2Value=0;

int sendMeasuredData = 1;
 
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
      Serial.println(client.state());
      delay(2000); 
    }
  }
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  //for MPU-6050
  Wire.begin(sda, scl);
  MPU6050_Init();
  pinMode(relayPinFan, OUTPUT);
  pinMode(relayPinBulb, OUTPUT);
  //pinMode(buzzerPin, OUTPUT);
  analogWrite(buzzerPin, LOW);
  digitalWrite(relayPinFan, LOW);
  client.subscribe(FAN_MAX_TEMP_SUB);
  client.subscribe(FAN_MIN_TEMP_SUB);
  client.subscribe(HEATING_MAX_TEMP_SUB);
  client.subscribe(HEATING_MIN_TEMP_SUB);
  client.subscribe(SWITCH_SUB);
  client.subscribe(ALARM_SUB);
  client.subscribe(RASP_FEEDBACK);
  analogWrite(heating_body, 0); //disable pwm at beginning
}

void callback(char* topic, byte* payload, unsigned int length) { 
  if (strcmp(topic,SWITCH_SUB)==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayPinBulb, LOW);
      //digitalWrite(relayPinFan, HIGH); //makni kasnije
      char onRelay[4] ="ON";
      client.publish(SWITCH_PUB, onRelay, true);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayPinBulb, HIGH);
      //digitalWrite(relayPinFan, LOW); //makni kasnije
      char offRelay[5] ="OFF";
      client.publish(SWITCH_PUB, offRelay, true);
    }
  }
  else if (strcmp(topic,ALARM_SUB)==0){
    if (!strncmp((char *)payload, "ON", length) && alarmFlag==false) {
      alarmFlag=true;
      Serial.println("Ukljucen alarm");
      char onRelay[4] ="ON";
      client.publish(ALARM_PUB, onRelay, true);
    }
    else if(!strncmp((char *)payload, "OFF", length) && alarmFlag==true){
      alarmFlag=false;
      Serial.println("Iskljucen alarm");
      char offRelay[4] ="OFF";
      client.publish(ALARM_PUB, offRelay, true);
    }
  }
  else if (strcmp(topic,FAN_MAX_TEMP_SUB)==0){
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
  else if(strcmp(topic,RASP_FEEDBACK)==0){
    Serial.println("Broker is ready");
    if(sendMeasuredData==3){
      Serial.println("Sada salji distance!");
      sendMeasuredData=2;
    }
    else if(sendMeasuredData==4){
      Serial.println("Sada salji temperature!");
      sendMeasuredData=1;
    }
  }
  Serial.println(topic);
  Serial.println();
  Serial.println("-----------------------"); 
}
 
void loop() {
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.println(WiFi.status());
  }
  while (!client.connected()) {
    Serial.println("Reconnecting to MQTT...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), brokerUsername, brokerPassword )) {
      Serial.println("Connected to broker");
      client.subscribe(FAN_MAX_TEMP_SUB);
      client.subscribe(FAN_MIN_TEMP_SUB);
      client.subscribe(HEATING_MAX_TEMP_SUB);
      client.subscribe(HEATING_MIN_TEMP_SUB);
      client.subscribe(SWITCH_SUB);
      client.subscribe(ALARM_SUB);
      client.subscribe(RASP_FEEDBACK);
      counterTemperature=0;
    }
    else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000); 
    }
  }
  double Ax, Ay, Az, T, Gx, Gy, Gz;
    Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);
  
    //divide each with their sensitivity scale factor
    Ax = (double)AccelX/AccelScaleFactor;
    Ay = (double)AccelY/AccelScaleFactor;
    Az = (double)AccelZ/AccelScaleFactor;
    T = (double)Temperature/340+36.53; //temperature formula
    Gx = (double)GyroX/GyroScaleFactor;
    Gy = (double)GyroY/GyroScaleFactor;
    Gz = (double)GyroZ/GyroScaleFactor;
    float temperature = (float)T;

    Serial.print("Ax: "); Serial.print(Ax);
    Serial.print(" Ay: "); Serial.print(Ay);
    Serial.print(" Az: "); Serial.print(Az);
    Serial.print(" T: "); Serial.print(T);
    Serial.print(" Gx: "); Serial.print(Gx);
    Serial.print(" Gy: "); Serial.print(Gy);
    Serial.print(" Gz: "); Serial.println(Gz);
    if((Gx > 6 || Gx < -6) && (Gy > 6 || Gy  < - 6) && (Gz > 6 || Gz  < - 6)){
      Serial.println("Potres!!!");
      tone(buzzerPin,1000, 1000);
    }
    if(T>=fanMaxTemp && digitalRead(relayPinFan)==0){
      Serial.println("Turn on Fan");
      digitalWrite(relayPinFan, HIGH);
    }
    else if (T<=fanMinTemp && digitalRead(relayPinFan)==1){
      Serial.println("Turn off Fan");
      digitalWrite(relayPinFan, LOW);
    }
  
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
 
    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);
 
    // Convert the time into a distance
    cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
    inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
    if(flagDistance == false){
      lastDistance = cm;
      flagDistance=true;
    }
    else if(flagDistance == true){
      if(abs(cm-lastDistance)>lastDistance*0.4 && counterDistance>=0 && counterDistance<=3){
        if(alarmFlag == true){
          Serial.println("Potencijalni provalnik");
        } 
        counterDistance++;
      }
      else if(abs(cm-lastDistance)>lastDistance*0.4 && counterDistance>=4){
        if(alarmFlag == true){
          Serial.println("Provalnik");
          tone(buzzerPin,500, 1000);
        }     
        counterDistance=0;
        flagDistance=false;
      }
      else{
        lastDistance = (lastDistance+cm)/2.0;
        counterDistance=0;
      }
    }
    if(T<=heatMaxTemp && T>=heatMinTemp){
      //turn on heater
      int getPWM = lineAValue*T+lineBValue;
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
    else if(T<heatMinTemp){
      analogWrite(heating_body, 1024);
    }
    else{
      //turn off heater
      analogWrite(heating_body, 0);
    }
  
  if(WiFi.status() == WL_CONNECTED && client.connected()){
    if(counterTemperature>=240){
      sprintf(temperatureChar, "%f", temperature);
      sprintf(distanceChar, "%f", cm);
      if(sendMeasuredData == 1){
        sendMeasuredData = 3;
        String message1 =  String("{temp: ") + temperatureChar + String(", topic: home/room2/temp}");
        message1.toCharArray(temperatureChar, 50);
        Serial.println("Saljem temperaturu...................................................................................");
        client.publish(TEMPERATURE_PUB, temperatureChar, true);
      }
      else if(sendMeasuredData == 2){
        sendMeasuredData = 4;
        counterTemperature=0;
        String message1 =  String("{dist: ") + distanceChar + String(", topic: home/room2/dist}");
        message1.toCharArray(distanceChar, 50);
        Serial.println("Saljem distance...................................................................................");
        client.publish(DISTANCE_PUB, distanceChar, true);
      }
    }
    if(counterTemperature>=480){
      if(sendMeasuredData==3){
        sendMeasuredData=2;
      }
      else if(sendMeasuredData==4){
        sendMeasuredData=1;
      }
    }
    Serial.println(counterTemperature);
    counterTemperature++;
    Serial.print(inches);
    Serial.print("in, ");
    Serial.print(cm);
    Serial.print("cm, ");
    Serial.print(lastDistance);
    Serial.print("cm");
    Serial.println();
    client.loop();
    //digitalWrite(relayPinFan, HIGH);
    delay(250);
  }
  
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.write(data);
  Wire.endTransmission();
}

// read all 14 register
void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress){
  Wire.beginTransmission(deviceAddress);
  Wire.write(regAddress);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, (uint8_t)14);
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050
void MPU6050_Init(){
  delay(150);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);//set +/-250 degree/second full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);// set +/- 2g full scale
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
