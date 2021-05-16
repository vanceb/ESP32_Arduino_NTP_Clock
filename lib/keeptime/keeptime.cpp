#include <keeptime.h>

#include <TimeLib.h>
#include <time.h>
#include <Timezone.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>


/* United Kingdom (London, Belfast) */
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};        // British Summer Time
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};         // Standard Time
Timezone UK(BST, GMT);


void keeptime(void * parameter) {
    WiFiUDP ntpUDP;
    int UTCOffset = 0;  // Set time to UTC - convert to local using Timezone
    NTPClient ntp(ntpUDP, NTP_SERVER, UTCOffset*60*60, UPDATE_INTERVAL*60*1000);

    ntp.begin();
    for(;;) {
        // Make sure WiFi is connected
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Unable to set NTP Time as wifi is not connected");
            Serial.println("Waiting for connection");
            while (WiFi.status() != WL_CONNECTED) {
                Serial.print(".");
                delay(1000);
            }
            Serial.println("\nConnected!");
        }

        if(ntp.update()) {
            setTime(ntp.getEpochTime());
            Serial.print(getEpochStringByParams(UK.toLocal(now())));
            Serial.println(" Time updated from ntp");
        }
        //Infinite loop - designed to run as an RTOS task
        Serial.print("*");
        delay(1000);
    }
}