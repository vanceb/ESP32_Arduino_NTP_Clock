#include <U8g2lib.h>
#include <FastLED.h>
#include<WiFi.h>

#include <clockDisplay.h>
#include <keeptime.h>

#include <math.h>
#include <TimeLib.h>

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ OLED_CLK, /* data=*/ OLED_DATA, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C

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
    for(int i=0; i<NUM_LEDS; i++){
        led_ring[i] = CRGB::WhiteSmoke;
    }
    FastLED.show();
    delay(1000);
    led_ring[0] = CRGB::Plum;
    for(int i=1; i<NUM_LEDS; i++){
        led_ring[i] = CRGB::Black;
    }
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

    int lastMinute = 0;
    for(;;) {
        // Get the fixed time for this loop to stop overruns
        time_t t = now();
    
        // Display time on the OLED if it has changed
        if (lastMinute != minute(t)) {
            u8g2.clearBuffer();					// clear the internal memory
            u8g2.setFont(u8g2_font_inr30_mn);	// choose a suitable font
            u8g2.drawStr(0,32,getEpochStringByParams(UK.toLocal(t), "%H:%M").c_str());
            lastMinute = minute(t);
            u8g2.sendBuffer();
        }
        // Clear the ring
        for(int i=0; i<NUM_LEDS; i++){
            led_ring[i] = CRGB::Black;
        }
        // Set the seconds
        int bright = 5;
        int sec = second(t);
        int led = sec / 5;
        int diff = abs(sec - (5 * led)) + 1;
        if((sec % 5) > 2) {
            led = (led + 1) % NUM_LEDS;
        }
        bright = bright / diff;
        led_ring[led] = CRGB(0,bright,0);
        Serial.print(bright);
        // Update led ring
        FastLED.show();
        
        // Put minimal delay just to allow other processes time
        // Software I2C for OLED is limiting frame rate...
        delay(d);
    }
}