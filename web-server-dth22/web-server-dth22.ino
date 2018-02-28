#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// DHT22 sensor library
#include "DHT.h"

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
