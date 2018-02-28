#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <fauxmoESP.h>

bool wemoState = false;
bool didWemoStateChange = false;

ESP8266WiFiMulti WiFiMulti;
fauxmoESP fauxmo;

const char* ssid = "UPCF197D7D";
const char* password = "pzc5Gvbyh7jr";

void setup() {
    Serial.begin(115200);

    connectToWifi();

    WiFiMulti.addAP(ssid, password);

    fauxmo.addDevice("tv");
    fauxmo.onMessage([](unsigned char device_id, const char * device_name, bool state) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
        
        wemoState = state;
        didWemoStateChange = true;
    });
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
}

void loop() {
    fauxmo.handle();

    if (didWemoStateChange) {
      Serial.printf("wemoState: %s\n", wemoState ? "true" : "false");
      didWemoStateChange = false;

      if (wemoState) {
        sendMessage("AAAAAQAAAAEAAAAVAw==");
      } else {
        sendMessage("AAAAAQAAAAEAAAAvAw==");
      }
    }
}

void sendMessage(char* command) {
  HTTPClient http;

  http.begin("http://192.168.0.157/sony/IRCC");
  http.addHeader("Content-Type", "text/xml; charset=UTF-8");
  http.addHeader("SOAPACTION", "\"urn:schemas-sony-com:service:IRCC:1#X_SendIRCC\"");
  http.addHeader("Cookie", "auth=88906E973C25FA1F19DB81459F7024D6B3FEDF72");
//  http.addHeader("X-Auth-PSK", "0000");

  char postMessage[320];
  
  sprintf(postMessage, "<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:X_SendIRCC xmlns:u=\"urn:schemas-sony-com:service:IRCC:1\"><IRCCCode>%s</IRCCCode></u:X_SendIRCC></s:Body></s:Envelope>", command);
  
  int httpCode = http.POST(postMessage);
  http.writeToStream(&Serial);
  http.end();
}

