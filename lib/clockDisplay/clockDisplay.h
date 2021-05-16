#ifndef CLOCKDISPLAY_H
#define CLOCKDISPLAY_H

/* General */
#define MAX_REFRESH_RATE        50
#define DISPLAY_REFRESH_RATE    10

/* OLED Display */
#define OLED_DATA   5
#define OLED_CLK    4

/* Neopixel Ring */
#define DATA_PIN    15
#define NUM_LEDS    12

/* Function prototypes */
void display(void *);  

#endif