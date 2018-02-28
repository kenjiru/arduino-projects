#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <fauxmoESP.h>

fauxmoESP fauxmo;

const char* ssid = "UPC0054202";
const char* password = "UYXJMSCD";

void setup() {
    Serial.begin(115200);

    connectToWifi();

    fauxmo.addDevice("tv");
    fauxmo.onMessage([](unsigned char device_id, const char * device_name, bool state) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");

        HTTPClient http;

        http.begin("http://tinkerman.cat");
        
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
    });
}

void sendMessage() {
  HTTPClient http;

  http.begin("http://192.168.0.19/sony/IRCC");
  http.addHeader("Content-Type", "text/xml; charset=UTF-8");
  http.addHeader("SOAPACTION", "urn:schemas-sony-com:service:IRCC:1#X_SendIRCC");
  http.addHeader("X-Auth-PSK", "0000");
  http.addHeader("NULL", "NULL");
  
  int httpCode = http.POST("<?xml version=\"1.0\"?><s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body><u:X_SendIRCC xmlns:u=\"urn:schemas-sony-com:service:IRCC:1\"><IRCCCode>AAAAAQAAAAEAAAASAw==</IRCCCode></u:X_SendIRCC></s:Body></s:Envelope>");
  http.writeToStream(&Serial);
  http.end();
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
}

