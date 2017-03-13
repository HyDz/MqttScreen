

/*
 * Mqtt Screen
 *
 * Created: 13/8/2016 
 * Author: HyDz
 *
 * Screen With SSD1306 display MQTT subscribtions
 */ 
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <ESP_SSD1306.h> 

#define OLED_RESET 16

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

ESP_SSD1306 display(OLED_RESET); // FOR I2C

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "SSID"
#define WLAN_PASS       "network password"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "mqtt broker adress"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "USERNAME"
#define AIO_KEY         "password"


/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_CLIENTID[] PROGMEM  = "ESP_CLIENTID";
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, AIO_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
// Setup a feed called 'onoff' for subscribing to changes.
const char TEMPERATURE_FEED[] PROGMEM = "place1/temperature";
Adafruit_MQTT_Subscribe temperature = Adafruit_MQTT_Subscribe(&mqtt, TEMPERATURE_FEED);

const char HUMIDITY_FEED[] PROGMEM = "place1/room/humidity";
Adafruit_MQTT_Subscribe humidity = Adafruit_MQTT_Subscribe(&mqtt, HUMIDITY_FEED);

const char PRESSURE_FEED[] PROGMEM = "place1/room/pressure";
Adafruit_MQTT_Subscribe pressure = Adafruit_MQTT_Subscribe(&mqtt, PRESSURE_FEED);

const char LUMIN_FEED[] PROGMEM = "place1/room/lumin";
Adafruit_MQTT_Subscribe lumin = Adafruit_MQTT_Subscribe(&mqtt, LUMIN_FEED);

char* temp;
char* humi;
char* pres;
char* lumi;

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
   display.begin(SSD1306_SWITCHCAPVCC);
  Serial.begin(115200);
  display.clearDisplay();
display.setTextSize(3);
display.setTextColor(WHITE);
display.setCursor(35,3);  
display.println("HyDz");
display.setTextSize(1);
display.setCursor(10,40);  
display.println("MQTT SCREEN");
display.display();
delay(500);

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
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(35,3); 
  display.println("Wifi Ok");
  display.setTextSize(1);
  display.setCursor(2,30);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(500);
  
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  
  

  // Setup MQTT subscription for onoff & slider feed.
  mqtt.subscribe(&temperature);
  mqtt.subscribe(&humidity);
  mqtt.subscribe(&pressure);
  mqtt.subscribe(&lumin);
}

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check if its the onoff button feed
    if (subscription == &temperature) {
      temp = (char *)temperature.lastread;
      Serial.println(temp);
      Serial.print(F("Température: "));
      Serial.println((char *)temperature.lastread);     
    }
    if (subscription == &humidity) {
      humi = (char *)humidity.lastread;
      Serial.println(humi);
      Serial.print(F("Humidité: "));
      Serial.println((char *)humidity.lastread);     
    }
    if (subscription == &pressure) {
      pres = (char *)pressure.lastread;
      Serial.println(pres);
      Serial.print(F("Pression: "));
      Serial.println((char *)pressure.lastread);     
    }
    if (subscription == &lumin) {
      lumi = (char *)lumin.lastread;
      Serial.println(lumi);
      Serial.print(F("Luminosité: "));
      Serial.println((char *)lumin.lastread);     
    }
    printScreen(temp, humi, pres, lumi);
  }

  // ping the server to keep the mqtt connection alive
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
        display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(2,20); 
  display.println("Try to connect MQTT");
  display.display();
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
   display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(35,3); 
  display.println("MQTT Ok");
  display.setTextSize(1);
  display.setCursor(2,30);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
}

void printScreen(String t, String h, String p,String l){
  display.clearDisplay();  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(5,0);
  display.print("Luminosite: ");
  display.println(l);
 // display.display();
  display.setCursor(5,8);
  display.print("Pression: ");
  display.print(p);
  display.println(" Pa");
  display.setTextSize(3);
  display.setCursor(5,17);
  display.println(t);
  display.setCursor(40,17);
  display.println("C");
  display.setCursor(5,44);
  display.println(h);
  display.setCursor(40,44);
  display.println("%");
  display.display();
  // delay(100);
 }
