load('api_config.js');
load('api_dht.js');
load('api_mqtt.js');
load('api_timer.js');

let topic = '/devices/' + Cfg.get('device.id') + '/events';
let dht = DHT.create(14, DHT.AM2302);  //14 is the pin which is connected from DHT22 to ESP8266

Timer.set(2000, true, function() {
  let msg = JSON.stringify({ t: dht.getTemp(), h: dht.getHumidity() });
  let ok = MQTT.pub(topic, msg, 1);
  print(ok, msg);
}, null);
