#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "UPC0054202";
const char* password = "UYXJMSCD";

// Does NOT work, for some reason WiFiMulti works fine..

void setup() {
    Serial.begin(115200);
    connectToWifi();
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
    if(WiFi.status() == WL_CONNECTED){   //Check WiFi connection status
      HTTPClient http;

      http.begin("http://213.157.176.2");
      
      int httpCode = http.GET();
    
      // httpCode will be negative on error
      if(httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    
          // file found at server
          if(httpCode == HTTP_CODE_OK) {
              String payload = http.getString();
              Serial.println(payload);
          }
      } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    
      http.end();
    }

    delay(30000);    //Send a request every 30 seconds
}

