#include <Arduino.h>
#include <ArduinoJson.h>
#include "debug.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <esp_sleep.h>
#include "wifimqtt.h"



#define DHTPIN 21
#define DHTTYPE DHT11
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 20       /* Time ESP32 will go to sleep (in seconds) */

// Objects
DHT dht(DHTPIN, DHTTYPE);

// Global Variables
uint32_t tempTime = millis();
uint32_t flowTime = millis();
byte data;
float temp;
float humidity;

// Function Prototypes
void displayTemperature();
void displayHumidity();
void sendESPdata();

void setup()
{
  Serial.begin(115200);
  Serial1.begin(9600);
  adc_attenuation_t::ADC_11db;
  connectAP();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();

  // Configure wake up source
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop()
{
  uint32_t now = millis();

  if(now - flowTime > 1000)
  {
    Serial1.write(0x0006);
    if(Serial1.available())
    {
      data = Serial1.read();
      debugln(data);
    }
  }

  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect("ESP32-");
  }
  // Enter deep sleep mode
   if(now - tempTime > 30000)
  {
    temp = dht.readTemperature();
    humidity = dht.readHumidity();

    displayTemperature();
    displayHumidity();
    sendESPdata();
    client.loop();
    tempTime = now;
    //client.disconnect();
    delay(500);
    //esp_deep_sleep_start();
  }
}

// Functions
void displayTemperature()
{
  char *msg = new char[255];
  sprintf(msg, "Temperature: %.2f", temp);
  debugln(msg);
  free(msg);
}

void displayHumidity()
{
  char *msg = new char[255];
  sprintf(msg, "Humidity: %.2f", humidity);
  debugln(msg);
  free(msg);
}
void sendESPdata()
{
  JsonDocument doc;

  doc["device"] = WiFi.macAddress();
  doc["temperature"] = temp;
  doc["humidity"] = humidity;
  doc["flowrate"] = data;

  doc.shrinkToFit(); // optional
  char buff[256];
  serializeJson(doc, buff);
  client.publish("ESPValues", buff);
}