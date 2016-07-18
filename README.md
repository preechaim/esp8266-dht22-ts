# esp8266-dth22-ts
Mini DIY weather sensor, ESP8266-01(ESP-01) connected with DHT22 temperature/humidity sensor, that publish data to ThingSpeak channel.

# Requirement
- ESP-01 Wifi module (or any compatible ESP8266 module)
- DHT22 temperature/humidity sensor (or DHT11)
- 3.3V Power source
- Wires, connectors, etc.
- Arduino IDE with
  - Board package: "esp8266" by ESP8266 Community 
  - Library package: "DHT sensor library" by Adafruit
- Thingspeak channel

# Instruction
1. Change Thingspeak API key, Wifi SSID and password. 
  - Also change the sensor type if you are not using DHT22.
2. Upload the code to ESP using Arduino IDE.
3. Connect ESP with DHT22 and power.
  - DHT22 data pin to ESP GPIO2.
  - 3.3V to ESP's Vcc, ESP's CH_PD and DHT's Vcc.
  - (recommend) Pull-up ESP's GPIO0,GPIO2,RST pins with 10K resistors.
  - (recommend) A 47uF capacitor between 3.3V and GND.
  - All GND to ground.
