#include <DHT.h>
#include <ESP8266WiFi.h>
ADC_MODE(ADC_VCC);

//// DHT
#define DHT_PIN 2
#define DHT_TYPE DHT22
#define DHT_PWR_PIN 3 // Override PIN3 Serial RX
#define DHT_DISABLE 1000
#define DHT_INTERVAL 2000

DHT dht(DHT_PIN, DHT_TYPE);
unsigned long nextRead = DHT_DISABLE + 2000; // wait for sensor to be stabilized
bool isRead = false;

//// Wifi connection
#define WIFI_SSID "WIFISSID"
#define WIFI_PASS "WIFIPASS"
#define WIFI_PRINT_INTERVAL 500

unsigned long nextWifiReport = 0;

//// Wifi client
WiFiClient client;
#define TS_HOST "api.thingspeak.com"
#define TS_PORT 80
#define TS_KEY "THINGSPEAKKEY"
#define TS_INTERVAL 300000

String postStr = "";
bool isConnected = false;

//// Max awake time
#define AWAKE_TIMEOUT 20000

//// RTC User Data
struct {
  uint32_t dhtErrorCount;
  uint32_t timeoutCount;
  uint32_t crc32;
} rtcData;

void setup(){
  Serial.begin(9600);
  pinMode(DHT_PWR_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  dht.begin();

  ESP.rtcUserMemoryRead(0, (uint32_t*) &rtcData, sizeof(rtcData));
  if (calcCRC32(((uint8_t*) &rtcData), sizeof(rtcData)-4) != rtcData.crc32){
    Serial.println("RTC data has wrong CRC, discarded.");
    rtcData.dhtErrorCount = 0;
    rtcData.timeoutCount = 0;
  } else {
    Serial.println("RTC data:");
    Serial.print("DHT Error Count:");
    Serial.println(rtcData.dhtErrorCount);
    Serial.print("Awake Timeout Count:");
    Serial.println(rtcData.timeoutCount);
  }

  Serial.println("Waking up.");

  // Power off to reset DHT
  digitalWrite(DHT_PWR_PIN, LOW);
  delay(DHT_DISABLE);
  digitalWrite(DHT_PWR_PIN, HIGH);
}

void loop(){
  if (AWAKE_TIMEOUT < millis()){
    Serial.println("Work is not finished, but it's time to sleep.");
    rtcData.timeoutCount++;
    goDeepSleep();
  }
  if (!isRead && nextRead < millis()){
    Serial.println("Reading...");

    /// DHT
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("DHT Read Error");
      rtcData.dhtErrorCount++;
      nextRead = millis()+DHT_INTERVAL;
      return;
    }
    float hi = dht.computeHeatIndex(t,h);
    Serial.print("Temperature(C):");
    Serial.println(t);
    Serial.print("Humidity(%):");
    Serial.println(h);
    Serial.print("HeatIndex(C):");
    Serial.println(hi);

    /// Input Voltage
    long vcc = ESP.getVcc();
    Serial.print("Vcc(mV):");
    Serial.println(vcc);

    Serial.print("DHT Error Count:");
    Serial.println(rtcData.dhtErrorCount);
    Serial.print("Awake Timeout Count:");
    Serial.println(rtcData.timeoutCount);

    postStr = "api_key=";
    postStr += TS_KEY;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
    postStr +="&field5=";
    postStr += String(hi);
    postStr +="&field6=";
    postStr += String(rtcData.timeoutCount);
    postStr +="&field7=";
    postStr += String(rtcData.dhtErrorCount);
    postStr +="&field8=";
    postStr += String(vcc);
    
    Serial.println(postStr);
    isRead = true;
    digitalWrite(DHT_PWR_PIN, LOW); // Power off
  }

  if (WiFi.status() != WL_CONNECTED){
    if (nextWifiReport < millis()){
      nextWifiReport = millis() + WIFI_PRINT_INTERVAL;
      Serial.print("Connecting WiFi:");
      Serial.println(WIFI_SSID);
    }
    return;
  }

  if (!isRead){
    return;
  }

  //// We can continue below,
  //// only if both sensor reading and wifi connection are completed.

  if (!isConnected){
    Serial.print("Connecting to ");
    Serial.print(TS_HOST);
    Serial.print(":");
    Serial.println(TS_PORT);
    if (client.connect(TS_HOST,TS_PORT)){
      
      Serial.print("Sending data...");
      
      client.println("POST /update HTTP/1.1");
      client.print("Host: ");
      client.println(TS_HOST);
      client.println("Connection: close");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.print("Content-Length: ");
      client.println(postStr.length());
      client.println();
      client.print(postStr);
      
      isConnected = true;
      Serial.println("Done");
    } else {
      Serial.println("Connection error");
      return;
    }
  }
  if (client.connected()){
    while (client.available()){
      Serial.print(client.read());
    }
	  return;
  }
  client.stop();
  Serial.println();
  Serial.println("Disconnected.");

  // Reset
  rtcData.dhtErrorCount = 0;
  rtcData.timeoutCount = 0;

  Serial.println("Work is done. Going back to sleep.");
  goDeepSleep();
}

void goDeepSleep(){
  rtcData.crc32 = calcCRC32(((uint8_t*) &rtcData), sizeof(rtcData)-4);
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtcData, sizeof(rtcData));

  Serial.print("Deep Sleep (us):");
  long sleepTime = (TS_INTERVAL - millis()) * 1000;
  if (sleepTime < 1){
    sleepTime = 1;
  }
  Serial.println(sleepTime);
  ESP.deepSleep(sleepTime);
}

uint32_t calcCRC32(const uint8_t *data, size_t length){
  uint32_t crc = 0xffffffff;
  while (length--){
    uint8_t c = *data++;
    for (uint32_t i=0x80; i > 0; i>>=1){
      bool bit = crc & 0x80000000;
      if (c & i){
        bit = !bit;
      }
      crc <<= 1;
      if (bit){
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}
