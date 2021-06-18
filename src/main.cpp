#include "Arduino.h"

#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiUdp.h> 
#include <TimeLib.h>
#include <Timezone.h>
#include <NTPClient.h>
#include <Wire.h>

#include <keeptime.h>
#include <clockDisplay.h>

// Uncomment to output the amount of spare task stack
//#define PRINT_SPARE_STACK

TaskHandle_t ntp_task;
TaskHandle_t display_task;

void setup()
{
  // Configure Serial port for debugging
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setting up...");

  // Create a task to display time and other data
  xTaskCreate(
    display,
    "Display_Task",
    1500,
    NULL,
    0,
    &display_task
  );

  // Set up Wifi - Using WiFi Manager
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "ESP Clock"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("ESP Clock");
  
  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  // Create a task to get time updates from NTP
  xTaskCreate(
    keeptime,
    "NTP_Task",
    1500,
    NULL,
    0,
    &ntp_task
  );
}

void loop()
{
  delay(1000);
  Serial.print("-");
  #ifdef PRINT_SPARE_STACK
  Serial.print("ntp spare stack: ");
  Serial.println(uxTaskGetStackHighWaterMark(ntp_task));
  Serial.print("oled spare stack: ");
  Serial.println(uxTaskGetStackHighWaterMark(display_task));
  #endif

  int spare = uxTaskGetStackHighWaterMark(ntp_task);
  if(spare < 100) {
    Serial.print("ntp_task low stack space: ");
    Serial.println(spare);
  }
  spare = uxTaskGetStackHighWaterMark(display_task);
  if(spare < 100) {
    Serial.print("oled_task low stack space: ");
    Serial.println(spare);
  }
}