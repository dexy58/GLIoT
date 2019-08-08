#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length);

int trigPin = 13;    // Trigger
int echoPin = 12;    // Echo
long duration, cm, inches;
long lastDistance;
boolean flagDistance = false;
int counterDistance = 0;

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

int relayPinBulb = 16;
int relayPinFan = 15;
int temperatureMax = 26;
int temperatureMin = 24;
int heating_body = 14;
char temperatureChar[10];
 
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

  Serial.print("Connected, IP address: ");
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
  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  //for MPU-6050
  Wire.begin(sda, scl);
  MPU6050_Init();
  pinMode(relayPinFan, OUTPUT);
  pinMode(relayPinBulb, OUTPUT);
  digitalWrite(relayPinFan, LOW);
  client.subscribe("home/device02/switchLight");
  analogWrite(heating_body, 0); //disable pwm at beginning
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  if (strcmp(topic,"home/device02/switchLight")==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayPinBulb, HIGH);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayPinBulb, LOW);
    }
  }
  else{
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
  }
  Serial.println();
  Serial.println("-----------------------"); 
}
 
void loop() {
  client.loop();
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
  if(Gx > 4 || Gx < -6 && Gy > 4 || Gy  < -6 && Gz > 4 || Gz  < -6){
    Serial.println("Potres!!!");
  }
  Serial.println(digitalRead(relayPinBulb));
  Serial.println(digitalRead(relayPinFan));
  if(T>=temperatureMax && digitalRead(relayPinFan)==0){
    Serial.println("Turn on Fan");
    digitalWrite(relayPinFan, HIGH);
  }
  else if (T<=temperatureMin && digitalRead(relayPinFan)==1){
    Serial.println("Turn of Fan");
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
      Serial.println("Potencijalni provalnik");
      counterDistance++;
    }
    else if(abs(cm-lastDistance)>lastDistance*0.4 && counterDistance>=4){
      Serial.println("Provalnik");
      counterDistance=0;
      flagDistance=false;
    }
    else{
      lastDistance = (lastDistance+cm)/2.0;
      counterDistance=0;
    }
  }
  if(T<=15){
    analogWrite(heating_body, 1024);
  }
  else if(T>15 && T<=19){
    analogWrite(heating_body, 613);
  }
  else if(T>19 && T<=22){
    analogWrite(heating_body, 204);
  }
  else if(T>22){
    analogWrite(heating_body, 0);
  }
  sprintf(temperatureChar, "%f", temperature);
  client.publish("home/device02/temperature", temperatureChar);
  Serial.print(inches);
  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm, ");
  Serial.print(lastDistance);
  Serial.print("cm");
  Serial.println();
  delay(250);
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
