load('api_config.js');
load('api_dht.js');
load('api_mqtt.js');
load('api_timer.js');

let deviceIDCustom ='01';
let topicTemp ='home/device' + deviceIDCustom +'/temperature';
let topicHum ='home/device' + deviceIDCustom +'/humidity';

let topic = '/devices/' + Cfg.get('device.id') + '/events';
let dht = DHT.create(14, DHT.AM2302); //14 is the pin which is connected from DHT22 to ESP8266

Timer.set(2000, true, function() {
  let messageTemp = JSON.stringify(dht.getTemp());
  let okTemp = MQTT.pub(topicTemp, messageTemp, 2); //use qos=2
  print('Published:', okTemp, topicTemp, '->', messageTemp);
  let messageHum = JSON.stringify(dht.getHumidity());
  let okHum = MQTT.pub(topicHum, messageHum, 2); //use qos=2
  print('Published:', okHum, topicHum, '->', messageHum);
  //let msg = JSON.stringify({ t: dht.getTemp(), h: dht.getHumidity() });
  //let ok = MQTT.pub(topic, msg, 1);
  //print(ok, msg);
}, null);
