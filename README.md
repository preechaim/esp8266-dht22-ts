# esp8266-dth22-ts
Mini DIY low-powered weather sensor, ESP8266-01(ESP-01) connected with DHT22 temperature/humidity sensor, that publish data to ThingSpeak channel.

# Requirement
- ESP-01 Wifi module (or any compatible ESP8266 module)  
- DHT22 temperature/humidity sensor (or DHT11)  
- 3.3V Power source  
- Wires, connectors, etc.  
- Arduino IDE with  
    - Board package: "esp8266" by ESP8266 Community https://github.com/esp8266/Arduino  
    - Library package: "DHT sensor library" by Adafruit (Version 1.2.3) https://github.com/adafruit/DHT-sensor-library/tree/1.2.3  
- Thingspeak channel  

# Instruction
1. Change Thingspeak API key, Wifi SSID and password.  
    - Also change the sensor type if you are not using DHT22.  
2. Hardwire ESP XPD_DCDC pin (bottom right corner of the mcu) to RST pin. This is neccessary for ESP-01 to wakeup from deepsleep.  
3. Upload the code to ESP using Arduino IDE.  
4. Connect ESP with DHT22 and power.  
    - DHT22 data pin to ESP GPIO2.  
    - DHT22 Vcc pin to ESP GPIO3(RX).  
    - 3.3V to ESP's Vcc, ESP's CH_PD.  
    - (recommend) Pull-up ESP's GPIO0,GPIO2,RST pins with 10K resistors.  
    - (recommend) A 47uF capacitor between 3.3V and GND.  
    - All GND to ground.  
  
![](esp01-dht22_bb.png?raw=true)

# Testing and result
The sensor wakes up, reads and sends data to Thingspeak server, then goes to deepsleep for 5 minutes and repeats.  

- Powered with 2x AA batteries (average alkaline, 3V.), directly connected without  regulator.  
It operated for about one week until the battery voltage is dropped below working level of DHT22.  
  
![](esp01-dht22-aa.png?raw=true)

