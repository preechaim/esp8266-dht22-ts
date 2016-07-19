#include <DHT.h>
#include <ESP8266WiFi.h>
ADC_MODE(ADC_VCC);

//// DHT
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
const int dhtRetryInverval = 2 * 1000;
long nextRead = 1000;
long dhtErrorCount = 0;
bool isRead = false;

//// Wifi connection
const char* wifiSsid = "WIFISSID";
const char* wifiPass = "WIFIPASS";
const long wifiReportInterval = 500;
long nextWifiReport = 0;

//// Wifi client
WiFiClient client;
const char* tsServer = "api.thingspeak.com";
const char* tsApiKey = "THINGSPEAKKEY";
const long tsInterval = 300 * 1000;
String postStr = "api_key=";
bool isConnected = false;

//// Max awake time
const long timeout = 20 *1000;
long timeoutCount = 0;

void doDeepSleep() {
  //ESP.rtcUserMemoryWrite(0,
  //  &dhtErrorCount, sizeof(dhtErrorCount));
  //ESP.rtcUserMemoryWrite(sizeof(dhtErrorCount),
  //  &timeoutCount, sizeof(timeoutCount));
  Serial.print("Deep Sleep (us):");
  long sleepTime = (tsInterval - millis()) * 1000;
  if (sleepTime < 1){ 
    sleepTime = 1;
  }
  Serial.println(sleepTime);
  ESP.deepSleep(sleepTime);
}

void setup() {
  Serial.begin(9600);
  //ESP.rtcUserMemoryRead(0,
  //  &dhtErrorCount, sizeof(dhtErrorCount));
  //ESP.rtcUserMemoryRead(sizeof(dhtErrorCount),
  //  &timeoutCount, sizeof(timeoutCount));
  dht.begin();
  WiFi.begin(wifiSsid, wifiPass);
  Serial.println("Waking up.");
}

void loop() {
  if (timeout < millis()){
    Serial.println("Work is not finished, but it's time to sleep.");
    timeoutCount += 1;
    doDeepSleep();
  }
  if (!isRead && nextRead < millis()){
    Serial.println("Reading...");

    /// DHT
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("DHT Read Error");
      dhtErrorCount++;
      nextRead = millis()+dhtRetryInverval;
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
    Serial.println(dhtErrorCount);
    Serial.print("Awake Timeout Count:");
    Serial.println(timeoutCount);

    postStr += tsApiKey;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
    postStr +="&field5=";
    postStr += String(hi);
    postStr +="&field6=";
    postStr += String(dhtErrorCount);
    postStr +="&field7=";
    postStr += String(timeoutCount);
    postStr +="&field8=";
    postStr += String(vcc);
    
    Serial.println(postStr);
    isRead = true;
  }    
  if (WiFi.status() != WL_CONNECTED) {
    if (nextWifiReport < millis()){
      nextWifiReport = millis() + wifiReportInterval;
      Serial.print("Connecting WiFi:");
      Serial.println(wifiSsid);
    }
    return;
  }
  if (!isRead){
    return;
  }
  if (!isConnected){
    Serial.print("Connecting to ");
    Serial.println(tsServer);
    if (client.connect(tsServer,80)) {
      
      Serial.print("Sending data...");
      
      client.println("POST /update HTTP/1.1");
      client.print("Host: ");
      client.println(tsServer);
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

  timeoutCount = 0;
  Serial.println("Work is done. Going back to sleep.");
  doDeepSleep();
}
