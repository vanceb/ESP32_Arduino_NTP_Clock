#include <U8g2lib.h>
#include <FastLED.h>
#include<WiFi.h>

#include <clockDisplay.h>
#include <keeptime.h>

#include <math.h>
#include <TimeLib.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ OLED_CLK, /* data=*/ OLED_DATA, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C

int clear(CRGB *leds, uint16_t num, CRGB colour) {
    for(int i=0; i<num; i++) {
        leds[i] = colour; 
    }
}

/*************************************************
 * show60 - light single led using input from 0 to 59
 * Only a single LED is lit, the brightness based on
 * how close the value is to the led position
 * so 1 o'clock led lights brightest for 5, lower for 4 and 6
 * lowest for 3 and 7
 *************************************************/
int show60(CRGB *leds, uint8_t num, CRGB colour) {
    int bright = 255;
    int led = num / 5;
    if((num % 5) > 2) {
        led += 1;
    }
    int diff = num - (5 * led);
    led %= NUM_LEDS;
    if(diff != 0) {
        colour /= (abs(diff) + 1);
    }
    leds[led] += colour;
}

int slide60(CRGB *leds, uint8_t num, CRGB colour) {
    int led = num / 5;
    int next_led = (led + 1) % NUM_LEDS;
    int remainder = num % 5;
    if(remainder == 0) {
        leds[led] += colour;
    } else {
        leds[led] += (colour / 5) * (5 - remainder);
        leds[next_led] += (colour / 5) * remainder;
    }
}

int showTime(CRGB *leds, time_t t) {
    /*
    show60(leds, (hour(t) * 5) + (minute(t) / 5), CRGB::Red);
    show60(leds, minute(t), CRGB::Blue);
    show60(leds, second(t), CRGB::Green); 
    */
    slide60(leds, ((hour(t) % 12) * 5) + (minute(t) / 12), CRGB::Red);
    slide60(leds, minute(t), CRGB::Blue);
    slide60(leds, second(t), CRGB::Green); 
}

void display(void * parameters) {

    // Calculate the delay needed to meet the refresh rate (approx)
    int r = DISPLAY_REFRESH_RATE;
    r = r < MAX_REFRESH_RATE ? r : MAX_REFRESH_RATE;
    long d = 1000 / r;

    Serial.print("d: ");
    Serial.println(d);
    Serial.print("r: ");
    Serial.println(r);

    // Sanity delay (Suggested by FastLED to stop LED driving interfering with uploads)
    delay(2000);

    // Set up the LED ring
    CRGB led_ring[NUM_LEDS];
    FastLED.addLeds<WS2812, DATA_PIN, GRB>(led_ring, NUM_LEDS);

    // Initialise the OLED display
    u8g2.begin();

    // Show Wifi not connected
    // Output notification to OLED
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_inr30_mn);
    u8g2.drawStr(0,32, "WiFi!");
    u8g2.sendBuffer();

    // Light LED Ring
    FastLED.setBrightness(25);
    for(int i=0; i<NUM_LEDS; i++){
        led_ring[i] = CRGB::WhiteSmoke;
    }
    FastLED.show();
    delay(1000);
    for(int i=0; i<NUM_LEDS; i++) {
        led_ring[i] = CRGB::Black;
    }
    led_ring[0] = CRGB::Purple;
    FastLED.show();

    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
        CRGB first = led_ring[NUM_LEDS - 1];
        for(int i=1; i<NUM_LEDS; i++) {
            led_ring[i] = led_ring[i-1];
        }
        led_ring[0] = first;
        FastLED.show();
    }

    int lastMinute = -1;
    for(;;) {
        // Manage loop delay, looking out for overruns
        // Works with the delay at the bottom of this loop
        unsigned long m = millis();
        unsigned long refresh = m + d;
        if(refresh < m) {
            // we have overrun the long counter
            refresh = d;
        }

        // Get the fixed time for this loop
        time_t t = UK.toLocal(now());
    
        // Display time on the OLED
        // Software I2C is slow so only update if changed
        if (lastMinute != minute(t)) {
            lastMinute = minute(t);
            u8g2.clearBuffer();					// clear the internal memory
            u8g2.setFont(u8g2_font_inr30_mn);	// choose a suitable font
            u8g2.drawStr(0,32,getEpochStringByParams(t, "%H:%M").c_str());
            u8g2.sendBuffer();
        }

        // Clear the ring
        clear(led_ring, NUM_LEDS, CRGB::Black);
        // Set the seconds
        showTime(led_ring, t);
        // Update led ring
        FastLED.show();
        
        // Account for loop processing time
        // Software I2C for OLED slow
        if(millis() < refresh) {
            delay(refresh - millis());
        }
    }
}