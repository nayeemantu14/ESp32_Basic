#include <Arduino.h>
#include <ArduinoJson.h>
#include "debug.h"
#include <esp_sleep.h>
#include "wifimqtt.h"
#include "OTA.h"
#include "FlowSensor.h"
#include "PressureSensor.h"
#include "UV.h"

//defines
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 20       /* Time ESP32 will go to sleep (in seconds) */
#define UPDATE_FREQ 5000
// Objects

// Global Variables
uint32_t tempTime = millis();
uint32_t flowTime = 0;
uint32_t reportTime = millis(); // Track the last report time
uint32_t blereportTime = millis();

byte data[32];
static float flowrate;
static uint16_t pressureCH1 = 0;
static uint16_t pressureCH2 = 0;
static uint16_t UVVoltage = 0;
static String UVLampOn;
static float lastReportedFlowrate = 0.0; // Track the last reported flow rate
bool isFlowAvailable = false;

extern WiFiManager wm;

byte readflowCommand[] = {0x10, 0x5B, 0xFD, 0x58, 0x16};

// Function Prototypes
void displayFlow();
void sendESPdata();
float flowToFloat(byte b1, byte b2, byte b3, byte b4);

void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1);
  adc_attenuation_t::ADC_11db;
  init_pressure_ch1();
  init_pressure_ch2();
  connectAP();
  enableOTA();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // Configure wake up source
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop()
{
  server.handleClient();
  uint32_t now = millis();

  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect("ESP32-");
  }

  if (now - flowTime > 10000)
  {
    flowTime = millis();
    isFlowAvailable = readFlowSensorData(readflowCommand, sizeof(readflowCommand), flowrate, data, sizeof(data));
     debugln(isFlowAvailable ? "Flow data available" : "Flow data not available");
  }

  // Report data every hour or when the flow rate changes
  if ((now - reportTime >= UPDATE_FREQ) || (isFlowAvailable && (flowrate != lastReportedFlowrate)))
  {
    reportTime = now;
    lastReportedFlowrate = flowrate;
    pressureCH1 = readPressure_ch1();
    pressureCH2 = readPressure_ch2();
    UVVoltage = readUV();
    if(UVVoltage >= 1200)
    {
      UVLampOn = "On";
    }
    else
    {
      UVLampOn = "Off";
    }
    displayFlow();
    sendESPdata();
    memset(data, 0, sizeof(uint8_t) * 32);
    client.loop();
  }
}

// Functions

void displayFlow()
{
  char *msg = new char[255];
  sprintf(msg, "Flowrate: %.3f", flowrate);
  debugln(msg);
  free(msg);
}

void sendESPdata()
{
  JsonDocument doc;

  doc["device"] = WiFi.macAddress();
  if (isFlowAvailable)
  {
    doc["flowrate"] = flowrate;
  }
  else
  {
    doc["flowrate"] = "no data";
  }
  doc["pressureCH1"] = pressureCH1;
  doc["pressureCH2"] = pressureCH2;
  doc["UVVoltage"] = UVVoltage;
  doc["UVLampOn"] = UVLampOn;
  doc.shrinkToFit(); // optional
  char buff[256];
  serializeJson(doc, buff);
  debugln(buff);
  client.publish("SmartTee", buff);
}
