#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "DHT.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// io.adafruit.com
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "kenjiru"
#define AIO_KEY         "c2e597c3094442c38693fb6c6780af32"

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

const char TEMP_FEED[] PROGMEM = AIO_USERNAME "/feeds/temp";
const char HUM_FEED[] PROGMEM = AIO_USERNAME "/feeds/temp";

// DHT22 sensor library
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

MDNSResponder mdns;
ESP8266WebServer server(80);

const char* ssid = "UPC0054202";
const char* password = "UYXJMSCD";

char sensorStr[80];

void setup() {
  Serial.begin(115200);

  dht.begin();

  connectToWifi();
  startServer();
}

void connectToWifi() {
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
}

void getTemp() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  char tempStr[10], humStr[10];
  dtostrf(t, 4, 2, tempStr);
  dtostrf(h, 4, 2, humStr);

  sprintf(sensorStr, "Temperature: %s Humidity: %s", tempStr, humStr);
}

void startServer() {  
  server.on("/", [](){
    getTemp();    
    Serial.println(sensorStr);

    char webPage[200];
    char pageTemplate[] = "<html><head><meta http-equiv='refresh' content='5'></head><body><h1>Sensor reading</h1><p>%s</p></body></html>";
    sprintf(webPage, pageTemplate, sensorStr);
    
    server.send(200, "text/html", webPage);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
