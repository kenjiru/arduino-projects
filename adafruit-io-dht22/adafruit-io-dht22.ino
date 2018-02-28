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

WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");
Adafruit_MQTT_Publish hum = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/humidity");

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

// DHT22 sensor library
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "UPCF197D7D";
const char* password = "pzc5Gvbyh7jr";

char sensorStr[80];

// Values read from sensor
float temp_f;
float hum_f;

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2300;              // interval at which to read sensor

void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();

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

int delayTime = 60000;  // Wait 1 minute before sending data to web
int startDelay = 0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Now we can publish stuff!
  if (millis() - startDelay < delayTime) {
    Serial.print (".");
  } else {
    temp_f = dht.readTemperature(false);
    hum_f = dht.readHumidity();

    if (isnan(temp_f) || isnan(hum_f)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      startDelay = millis();

      char tempStr[10], humStr[10];
      dtostrf(temp_f, 4, 2, tempStr);
      dtostrf(hum_f, 4, 2, humStr);

      sprintf(sensorStr, "\nTemperature: %s Humidity: %s ", tempStr, humStr);
      Serial.print(sensorStr);
      
      // Publish to Adafruit
      if (!temp.publish(temp_f) || !hum.publish(hum_f)) {
        Serial.println("Failed!");
      } else {
        Serial.println("Sent!");
      }
    }
  }
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  if(!mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
