#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <DHTesp.h>
#include <ESP8266WiFi.h>

DHTesp dht;
WiFiClient espClient;
PubSubClient client(espClient);

char temperatureChar[10];
char humidityChar[10];

int relayPin = 15;

void callback(char* topic, byte* payload, unsigned int length);

#define RST_PIN         0           // Configurable, see typical pin layout above
#define SS_PIN          4          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

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
  pinMode(relayPin, OUTPUT);
  dht.setup(2, DHTesp::DHT22);
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
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522.PICC_IsNewCardPresent()){
        //return;
    }
    // Select one of the cards
    else if ( ! mfrc522.PICC_ReadCardSerial()){
        //return;
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
          client.publish("home/device03/switchLight", onRelay);          
        }
        else if(digitalRead(relayPin)==HIGH){
          Serial.println("Turning light OFF!");
          digitalWrite(relayPin, LOW);
          char offRelay[5] ="OFF";
          client.publish("home/device03/switchLight", offRelay);
        }
      }
      else{
        Serial.println("Access DENIED!");
      }
    }     
    TempAndHumidity measurement = dht.getTempAndHumidity(); 
    Serial.print("Temperature: ");
    Serial.println(measurement.temperature);
    Serial.print("Humidity: ");
    Serial.println(measurement.humidity);
    //dtostrf(measurement.temperature,5,2,temperatureChar);
    //dtostrf(measurement.humidity,5,2,humidityChar); 
    //Serial.println(dtostrf(measurement.humidity,5,2,humidityChar));
    sprintf(temperatureChar, "%f", measurement.temperature);
    sprintf(humidityChar, "%f", measurement.humidity);
    Serial.println(temperatureChar);
    Serial.println(humidityChar);
    client.publish("home/device03/temperature", temperatureChar);
    client.publish("home/device03/humidity", humidityChar);
    delay(2000);
}

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
