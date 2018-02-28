#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

MDNSResponder mdns;

// Replace with your network credentials
const char* ssid = "UPC0054202";
const char* password = "UYXJMSCD";

ESP8266WebServer server(80);

String webPage = "";

void setup() {
  Serial.begin(9600);

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

void startServer() {
  webPage += "<h1>Hello world!</h1>";
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
