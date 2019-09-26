#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

char temperatureChar[50];
char humidityChar[50];

int relayPin = 15;

void callback(char* topic, byte* payload, unsigned int length);

#define RST_PIN         0           // Configurable, see typical pin layout above
#define SS_PIN          4          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

const char* ssid = "SmartHuawei";              //WiFi variables
const char* password =  "smartstudent";

const char* mqttServer = "192.168.43.24";    //MQTT variables
const int mqttPort = 1883;

const char* brokerUsername = "openhabian";
const char* brokerPassword = "openhabian";

int counterDHT22 = 0;

const char TEMPERATURE_PUB[40] = "home/room3/temp";
const char HUMIDITY_PUB[40] = "home/room3/hum";
const char SWITCH_SUB[40]="home/room3/switchLight";
const char RASP_FEEDBACK[40]="rasp/feedback03";

int sendDHTData = 1;

void setup() {
    delay(5000);
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
    Serial.print(F("Using key (for A and B):"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();

    Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
    WiFi.begin(ssid, password);

    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println();

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("device03", brokerUsername, brokerPassword )) {
      Serial.println("connected");  
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000); 
    }
  }
  pinMode(relayPin, OUTPUT);
  dht.setup(2, DHTesp::DHT22);
  client.subscribe(SWITCH_SUB);
  client.subscribe(RASP_FEEDBACK);
}

void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  if (strcmp(topic,SWITCH_SUB)==0){
    if (!strncmp((char *)payload, "ON", length)) {
      Serial.print("ON");
      digitalWrite(relayPin, HIGH);
    }
    else{
      Serial.print("OFF");
      digitalWrite(relayPin, LOW);
    }
  }
  else if(strcmp(topic,RASP_FEEDBACK)==0){
    Serial.println("Broker is ready");
    if(sendDHTData==3){
      Serial.println("Sad posalji humidity");
      sendDHTData=2;
    }
    else if(sendDHTData==4){
      Serial.println("Sad posalji temperature");
      sendDHTData=1;
    }
  }
  Serial.println();
  Serial.println("-----------------------"); 
}

void loop() {
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.println(WiFi.status());
      sendDHTData=1;
    }
    while (!client.connected() && WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnecting to MQTT...");
      String clientId = "ESP8266-";
      clientId += String(random(0xffff), HEX);
      Serial.println(clientId);
      if (client.connect(clientId.c_str(), brokerUsername, brokerPassword )) {
        Serial.println("Connected to broker");
        sendDHTData=1;
        client.subscribe(RASP_FEEDBACK);
        client.subscribe(SWITCH_SUB);
      }
      else {
        Serial.print("failed with state ");
        Serial.println(client.state());
        delay(2000); 
      }
    }
    //Serial.println(client.connected());
    if(WiFi.status() == WL_CONNECTED && client.connected()){
      if ( ! mfrc522.PICC_IsNewCardPresent()){
      }
      else if ( ! mfrc522.PICC_ReadCardSerial()){
      }
      else{
        Serial.print(F("Card UID:"));
        dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
        String UIDstring;
        UIDstring = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          UIDstring = UIDstring + String(mfrc522.uid.uidByte[i]);
        }
        Serial.println();
        Serial.println(UIDstring);
        if(UIDstring == "575593" || UIDstring=="165302522"){
          Serial.println("Access GRANTED!");
          if(digitalRead(relayPin)== LOW){
            Serial.println("Turning light ON!");
            digitalWrite(relayPin, HIGH);
            char onRelay[4] ="ON";
            client.publish(SWITCH_SUB, onRelay, true);          
          }
          else if(digitalRead(relayPin)==HIGH){
            Serial.println("Turning light OFF!");
            digitalWrite(relayPin, LOW);
            char offRelay[5] ="OFF";
            client.publish(SWITCH_SUB, offRelay, true);
          }
        }
        else{
          Serial.println("Access DENIED!");
        }
      }     
      if(strcmp(dht.getStatusString(),"OK")==0){
        //Serial.println("DHT22 is OK!");
        if(counterDHT22 >= 240){
          //Serial.println("DHT22 is OK! and counter>240");
          TempAndHumidity measurement = dht.getTempAndHumidity();
          sprintf(temperatureChar, "%f", measurement.temperature);
          sprintf(humidityChar, "%f", measurement.humidity);
          if(sendDHTData == 1){
            sendDHTData=3;
            Serial.print("Temperature: ");
            Serial.println(measurement.temperature);
            String message1 =  String("{temp: ") + temperatureChar + String(", topic: home/room3/temp}");
            message1.toCharArray(temperatureChar, 50);
            client.publish(TEMPERATURE_PUB, temperatureChar, true);        
          }
          else if(sendDHTData == 2){
            sendDHTData=4;
            counterDHT22=0;
            Serial.print("Humidity: ");
            Serial.println(measurement.humidity);
            String message1 =  String("{hum: ") + humidityChar + String(", topic: home/room3/hum}");
            message1.toCharArray(humidityChar, 50);
            client.publish(HUMIDITY_PUB, humidityChar, true);     
          }
        }
      }
      else{
        //Serial.println("DHT22 is not OK!");
        //Serial.println(dht.getStatusString());
        if(counterDHT22 >= 240){
          //Serial.println("DHT22 is not OK! and counter>240");
          if(sendDHTData == 1){
            sendDHTData=3;
            Serial.println("Temperature: -99");
            String message1 =  String("{temp: ") + "-99" + String(", topic: home/room3/temp}");
            message1.toCharArray(temperatureChar, 50);
            client.publish(TEMPERATURE_PUB, temperatureChar, true);        
          }
          else if(sendDHTData == 2){
            sendDHTData=4;
            counterDHT22=0;
            Serial.println("Humidity: -99");
            String message1 =  String("{hum: ") + "-99" + String(", topic: home/room3/hum}");
            message1.toCharArray(humidityChar, 50);
            client.publish(HUMIDITY_PUB, humidityChar, true);
            counterDHT22=0;         
          }
        }
      }
   Serial.println(counterDHT22);
   counterDHT22++;
   delay(250);
  }
  if(counterDHT22>=480){
    if(sendDHTData==3){
      sendDHTData=2;
    }
    else if(sendDHTData==4){
      sendDHTData=1;
    }
  }
  client.loop();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
