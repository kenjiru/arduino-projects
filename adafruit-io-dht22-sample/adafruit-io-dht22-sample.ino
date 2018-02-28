/* Sketch to collect temp and send via MQTT to web.  

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino


  MQTT code Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
  
*/


#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <DHT.h>


/* DHT SENSOR SETUP */

#define DHTTYPE DHT22
#define DHTPIN  2

/* WIFI SETUP */

#define WLAN_SSID       "UPC0054202"  //Put your SSID here
#define WLAN_PASS       "UYXJMSCD"  //Put you wifi password here

/* ADAFRUIT IO SETUP */

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "kenjiru"      //Put your Adafruit userid here
#define AIO_KEY         "c2e597c3094442c38693fb6c6780af32"    //Put your Adafruit IO key here

DHT dht(DHTPIN, DHTTYPE); 
 
float temp_f;  // Values read from sensor
String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2300;              // interval at which to read sensor

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/************************* Feeds ***************************************/

// Setup a feed called 'temp' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature");

/********************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();


void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();           // initialize temperature sensor

  Serial.println(F("MQTT Temp"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Get initial temp

  temp_f = dht.readTemperature(true);
  Serial.println();
  Serial.print("Initial Temp: ");
  Serial.println(temp_f);
  Serial.println();
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
    Serial.print(".");
  } else {
    temp_f = dht.readTemperature();               //Get temp in Farenheit
    startDelay = millis();
    Serial.print(F("\nSending temp: "));
    Serial.print(temp_f);
    Serial.print("...");
    
    if (! temp.publish(temp_f)) {                     //Publish to Adafruit
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("Sent!"));
    }
  }
  
  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds

  if(! mqtt.ping()) {
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
