load('api_config.js');
load('api_events.js');
load('api_gpio.js');
load('api_mqtt.js');
load('api_net.js');
load('api_sys.js');
load('api_timer.js');
load('api_dht.js');

let relay_pin = 0;  // Sonoff relay pin
// init hardware
GPIO.set_mode(relay_pin, GPIO.MODE_OUTPUT);
GPIO.write(relay_pin, 0);  // default to off


let deviceIDCustom ='01';
let topicTemp ='home/device' + deviceIDCustom +'/temperature';
let topicHum ='home/device' + deviceIDCustom +'/humidity';

let topic = '/devices/' + Cfg.get('device.id') + '/events';
let dht = DHT.create(14, DHT.AM2302); //14 is the pin which is connected from DHT22 to ESP8266

MQTT.sub('home/device01/switchLight', function(conn, topic, msg) {
  print('Topic:', topic, 'message:', msg);
  if(msg === 'ON'){
	  print('Switch is ON');
	  GPIO.write(relay_pin,1);
  }
  if(msg === 'OFF'){
	  print('Switch is OFF');
	  GPIO.write(relay_pin, 0);
  }
  //radi
}, null);

Timer.set(3000, true, function() {
  let messageTemp = JSON.stringify(dht.getTemp());
  let okTemp = MQTT.pub(topicTemp, messageTemp, 2); //use qos=2
  print('Published:', okTemp, topicTemp, '->', messageTemp);
  let messageHum = JSON.stringify(dht.getHumidity());
  let okHum = MQTT.pub(topicHum, messageHum, 2); //use qos=2
  print('Published:', okHum, topicHum, '->', messageHum);
  //let msg = JSON.stringify({ t: dht.getTemp(), h: dht.getHumidity() });
  //let ok = MQTT.pub(topic, msg, 1);
  //print(ok, msg);
  print ('Device id: ', Cfg.get('device.id'));
}, null);
