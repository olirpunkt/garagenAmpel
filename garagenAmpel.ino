#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"

//Pin for Neopixel Strip
#define PIN 12

#ifdef __AVR__
  #include <avr/power.h>
#endif

WiFiClient espClient;
PubSubClient client(espClient);

//JSON Document for MQTT Messages
StaticJsonDocument<256> doc;
String status;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

int rCol = 0;
int gCol = 150;
int bCol = 0;
int brightness = 150;

//callback function for MQTT subscription
void callback(char * topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived: ");
  Serial.println(topic);
  deserializeJson(doc, payload, length);
}


void setup() {
  Serial.begin(115200);
  Serial.println("setup");
  
  strip.begin();
  strip.setBrightness(50);

  Serial.println("dazwischen");

  strip.show(); // Initialize all pixels to 'off'
  
  setup_wifi();

  //after WIFI start MQTT connection
  client.setServer(MQTT_BROKER, 1883);
  
  Serial.println("setup done");

}

void setup_wifi() {
  String newHostname = "TrafficMCU";
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.hostname(newHostname.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setCallback(callback);
}


// keep MQTT-Subscription online
void reconnect() {
  while (!client.connected()) {
    Serial.print("Reconnecting...");
    if (!client.connect("TrafficLight", "cheffe", "wikiwins")) {
      Serial.print("Failed, rd=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    } else {
      Serial.println("Connected");
      client.publish("Traffic", "I'm alive");
      client.subscribe("/home/garage/door");
    }
  }
}

void setRing() {
  
  //int status = 0;

  if (status.equals("open")) {
    int i = 0;
    while (i < 12)
    {
      strip.setPixelColor(i, strip.Color(255,0,0));
      strip.setBrightness(40);
      strip.show();
      i = i + 2;
    }
  } 
  else if (status.equals("closed")) {
    int i = 1;
    while (i < 12) 
    {
      strip.setPixelColor(i, strip.Color(0,255,0));
      strip.setBrightness(40);
      strip.show();
      i = i + 2;
    }
  } else {
    for (int i = 0; i < 12; i++) {
      strip.setPixelColor(i, strip.Color(125,125,125));
      strip.setBrightness(40);
      strip.show();
    }
  }
}



void loop() {
  
  if (!client.connected()) {
    reconnect();
  }

  strip.clear();
  client.loop();

  String tmpStatus = doc["status"];

  if (!tmpStatus.equals(status)) 
  {
    Serial.print(status);
    Serial.println("new status");
    status = tmpStatus;
    setRing();
  } else {
    Serial.println("no new status");
  }
  

  delay(1000);

}
