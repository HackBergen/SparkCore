// This #include statement was automatically added by the Spark IDE.
#include "SparkCorePolledTimer/SparkCorePolledTimer.h"

/*-------------------------------------------------------------------------
  Spark Core library to control WS2811/WS2812 based RGB
  LED devices such as Adafruit NeoPixel strips.
  Currently handles 800 KHz and 400kHz bitstream on Spark Core, 
  WS2812, WS2812B and WS2811.

  Also supports:
  - Radio Shack Tri-Color Strip with TM1803 controller 400kHz bitstream.
  - TM1829 pixels
  
  PLEASE NOTE that the NeoPixels require 5V level inputs 
  and the Spark Core only has 3.3V level outputs. Level shifting is
  necessary, but will require a fast device such as one of the following:

  [SN74HCT125N]
  http://www.digikey.com/product-detail/en/SN74HCT125N/296-8386-5-ND/376860

  [SN74HCT245N] 
  http://www.digikey.com/product-detail/en/SN74HCT245N/296-1612-5-ND/277258

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.
  Modified to work with Spark Core by Technobly.
  Contributions by PJRC and other members of the open source community.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!
  --------------------------------------------------------------------*/

/* ======================= includes ================================= */


#include "application.h"
//#include "spark_disable_wlan.h" (for faster local debugging only)
#include "neopixel/neopixel.h"

/* ======================= prototypes =============================== */

void colorAll(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);
boolean LEDflag = FALSE;
#define LED D7 // Tiny blue LED onboard

/* ======================= extra-examples.cpp ======================== */

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 10
#define PIXEL_PIN A7
#define PIXEL_TYPE WS2812B

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
//               note: if not specified, D2 is selected for you.
// Parameter 3 = pixel type [ WS2812, WS2812B, WS2811, TM1803 ]
//               note: if not specified, WS2812B is selected for you.
//               note: RGB order is automatically applied to WS2811,
//                     WS2812/WS2812B/TM1803 is GRB order.
//
// 800 KHz bitstream 800 KHz bitstream (most NeoPixel products ...
//                         ... WS2812 (6-pin part)/WS2812B (4-pin part) )
//
// 400 KHz bitstream (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//                   (Radio Shack Tri-Color LED Strip - TM1803 driver
//                    NOTE: RS Tri-Color LED's are grouped in sets of 3)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

SYSTEM_MODE(SEMI_AUTOMATIC);

SparkCorePolledTimer updateTimer(10000);  //Create a timer object and set it's timeout in milliseconds
void OnTimer(void);   //Prototype for timer callback method

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(LED, OUTPUT); // Tiny blue LED onboard
  digitalWrite(LED, LOW);   // Turn Off the LED
  updateTimer.SetCallback(OnTimer);
}

void loop() {
  updateTimer.Update();
  
  // Some example procedures showing how to display to the pixels:
  // Do not run more than one of these at a time, or the b/g tasks 
  // will be blocked.
  //--------------------------------------------------------------
  
  pixelSwipeRandom(strip.Color(0, 0, 10),strip.Color(15, 50, 0), 20);

  //pixelWipe(strip.Color(0, 0, 0),strip.Color(0, 0, 15), 50); // Red
  //pixelWipeReverse(strip.Color(0, 0, 0),strip.Color(0, 0, 15), 50); 
  //colorWipe(strip.Color(255, 0, 0), 50); // Red
  
  //colorWipe(strip.Color(20, 25, 0), 50); // Green
  
  //colorWipe(strip.Color(0, 0, 25), 50); // Blue
  
  //rainbow(20);
  
  //rainbowCycle(20);
  
  //colorAll(strip.Color(0, 255, 255), 50); // Magenta
}

void pixelSwipeRandom(uint32_t c, uint32_t b, uint8_t wait) {
  pixelWipe(c, b, wait); // Red
  clearAll(b);
  strip.show();
  delay(random(100,10000));
  pixelWipeReverse(c,b, wait); 
  clearAll(b);
  strip.show();
  delay(random(100,10000));
}
// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;
  
  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}

// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void pixelWipe(uint32_t c, uint32_t b, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    clearAll(b);
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);

  }
}

void pixelWipeReverse(uint32_t c, uint32_t b, uint8_t wait) {
  for(int16_t i=strip.numPixels() -1; i>=0; i--) {
    clearAll(b);
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void clearAll(uint32_t b) {
  for(uint16_t t=0; t<strip.numPixels(); t++) {
    strip.setPixelColor(t, b);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

void OnTimer(void) {  //Handler for the timer, will be called automatically
  if (LEDflag == FALSE) {
    digitalWrite(LED, HIGH);   // Turn ON the LED
    LEDflag = TRUE;
  } else {
    digitalWrite(LED, LOW);   // Turn OFF the LED
    LEDflag = FALSE;
  }
  if (Spark.connected() == false) {
    Spark.connect();
  }
}

