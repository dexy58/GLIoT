#include <SPI.h>

#include <deprecated.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>
#include <require_cpp11.h>

#include <PubSubClient.h>

/*
 * pin layout used:
 * ----------------------------------
 *             MFRC522      Node     
 *             Reader/PCD   MCU      
 * Signal      Pin          Pin      
 * ----------------------------------
 * RST/Reset   RST          GPIO0        
 * SPI SS      SDA(SS)      GPIO4       
 * SPI MOSI    MOSI         GPIO13
 * SPI MISO    MISO         GPIO12
 * SPI SCK     SCK          GPIO14
 * 3.3V        3.3V         3.3V
 * GND         GND          GND
 */

#include "DHTesp.h"
#include <ESP8266WiFi.h>

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

char temperatureChar[10];
char humidityChar[10];

int relayPin = 2;

void callback(char* topic, byte* payload, unsigned int length);

#define RST_PIN 0
#define SS_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
   Serial.println();

  WiFi.begin("SmartHomeAP", "smarthome");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
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
 
    if (client.connect("device03", "openhabian", "openhabian" )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println(F("*****************************"));
  Serial.println(F("MFRC522 Digital self test"));
  Serial.println(F("*****************************"));
  mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader
  Serial.println(F("-----------------------------"));
  Serial.println(F("Only known versions supported"));
  Serial.println(F("-----------------------------"));
  Serial.println(F("Performing test..."));
  bool result = mfrc522.PCD_PerformSelfTest(); // perform the test
  Serial.println(F("-----------------------------"));
  Serial.print(F("Result: "));
  if (result)
    Serial.println(F("OK"));
  else
    Serial.println(F("DEFECT or UNKNOWN"));
  Serial.println();

  pinMode(relayPin, OUTPUT);
  
  dht.setup(16, DHTesp::DHT22);
  client.subscribe("home/device03/switchLight");
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  if (strcmp(topic,"home/device03/switchLight")==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayPin, HIGH);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayPin, LOW);
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
  
  TempAndHumidity measurement = dht.getTempAndHumidity();
 
  Serial.print("Temperature: ");
  Serial.println(measurement.temperature);
 
  Serial.print("Humidity: ");
  Serial.println(measurement.humidity);

  dtostrf(measurement.temperature,5,2,temperatureChar);
  dtostrf(measurement.humidity,5,2,humidityChar);
  
  client.publish("home/device03/temperature", temperatureChar);
  client.publish("home/device03/humidity", humidityChar);

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    unsigned long uid = getID();
    if(uid != -1){
      Serial.print("Card detected, UID: "); Serial.println(uid);
    }
    //delay(50);
    //return;
  }
  else if ( ! mfrc522.PICC_ReadCardSerial()) {
    //delay(50);
    //return;
  }
  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid)); 
  delay(2000); 
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

unsigned long getID(){
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return -1;
  }
  unsigned long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}
