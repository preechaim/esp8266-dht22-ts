#include <DHT.h>
#include <ESP8266WiFi.h>

ADC_MODE(ADC_VCC);

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
const int dhtRetryInterval = 2 * 1000;
long dhtErrorCount = 0;

WiFiClient client;
const char* wifiSsid = "WIFISSID";
const char* wifiPass = "WIFIPASS";
const char* tsServer = "api.thingspeak.com";
const char* tsApiKey = "THINGSPEAKKEY";
const long tsInterval = 300 * 1000;
long nextTsUpdate = 0;

const long wifiInterval = 500;
long lastWifi = 0;
bool isConnected = false;

void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.begin(wifiSsid, wifiPass);
  delay(2000);
  Serial.println("Start");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (lastWifi + wifiInterval < millis()){
      lastWifi = millis();
      Serial.print("Connecting WiFi:");
      Serial.println(wifiSsid);
    }
    return;
  }
  if (nextTsUpdate < millis()){
    client.stop();
    isConnected = false;
    Serial.println("Reading...");

    /// DHT
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("DHT Read Error");
      dhtErrorCount++;
      nextTsUpdate = millis()+dhtRetryInterval;
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

    String postStr = "api_key=";
    postStr += tsApiKey;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
    postStr +="&field6=";
    postStr += String(hi);
    postStr +="&field7=";
    postStr += String(dhtErrorCount);
    postStr +="&field8=";
    postStr += String(vcc);
    
    dhtErrorCount = 0;
    nextTsUpdate = millis() + tsInterval;
    
    Serial.println(postStr);
      
    Serial.print("Connecting to ");
    Serial.println(tsServer);
    if (client.connect(tsServer,80)) {
      isConnected = true;
      
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
      
      Serial.println("Done");
    } else {
      Serial.println("Connection error");
    }
    return;
  }
  if (isConnected){
    if (client.connected()){
      while (client.available()){
        Serial.print(client.read());
      }
    } else {
      client.stop();
      Serial.println();
      Serial.println("Disconnected.");
      isConnected = false;
    }
  }
}


