// This #include statement was automatically added by the Spark IDE.
#include "lib8tion.h"
// From https://raw.githubusercontent.com/FastLED/FastLED/master/lib8tion.h
// Also add https://raw.githubusercontent.com/FastLED/FastLED/master/lib8tion.cpp
// Use the same names lib8tion.h and lib8tion.cpp

// This #include statement was automatically added by the Spark IDE.
#include "neopixel/neopixel.h"

#define PIXEL_COUNT 10
#define PIXEL_PIN A7
#define PIXEL_TYPE WS2812B
boolean LEDflag = FALSE;
#define LED D7 // Tiny blue LED onboard

#define BRIGHTNESS  30
#define FRAMES_PER_SECOND 60

uint32_t leds[PIXEL_COUNT];

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

//Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(BRIGHTNESS);
  pinMode(LED, OUTPUT); // Tiny blue LED onboard
  digitalWrite(LED, LOW);   // Turn Off the LED
}

void loop() {
  // Add entropy to random number generator; we use a lot of it.
  random16_add_entropy( random(1,65500));
  Fire2012();
  strip.show();
  delay(1000 / FRAMES_PER_SECOND);
}

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//
// Adapted for a few leds on the Spark Core with Adafruit_NeoPixel as we wait for the FastLED library to be ported
//
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  5

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 4

void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[PIXEL_COUNT];
  uint8_t r1,g1,b1;

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < PIXEL_COUNT; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / PIXEL_COUNT) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= PIXEL_COUNT - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < PIXEL_COUNT; j++) {
        leds[j] = HeatColor( heat[j]);
         r1 = (uint8_t)(leds[j] >> 16),
         g1 = (uint8_t)(leds[j] >>  8),
         b1 = (uint8_t)(leds[j] >>  0);
        strip.setPixelColor(j,r1,g1,b1);
    }
}


// CRGB HeatColor( uint8_t temperature)
// [to be included in the forthcoming FastLED v2.1]
//
// Approximates a 'black body radiation' spectrum for 
// a given 'heat' level.  This is useful for animations of 'fire'.
// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
// This is NOT a chromatically correct 'black body radiation' 
// spectrum, but it's surprisingly close, and it's extremely fast and small.
//
// On AVR/Arduino, this typically takes around 70 bytes of program memory, 
// versus 768 bytes for a full 256-entry RGB lookup table.

uint32_t HeatColor( uint8_t temperature)
{
  //uint32_t heatcolor;
  uint8_t r1,g1,b1;
  
  // Scale 'heat' down from 0-255 to 0-191,
  // which can then be easily divided into three
  // equal 'thirds' of 64 units each.
  uint8_t t192 = scale8_video( temperature, 192);

  // calculate a value that ramps up from
  // zero to 255 in each 'third' of the scale.
  uint8_t heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  uint8_t hottest = t192 & 0x80;
  uint8_t middle = t192 & 0x40; 
 
  // now figure out which third of the spectrum we're in:
  if (hottest > 0) {
    // we're in the hottest third
    r1 = 255; // full red
    g1 = 255; // full green
    b1 = heatramp; // ramp up blue
  } 
//  else if ( t192 & 0x40 ) {
  else if ( middle > 0 ) {
    // we're in the middle third
    r1 = 255; // full red
    g1 = heatramp; // ramp up green
    b1 = 0; // no blue
    
  } 
  else {
    // we're in the coolest third
    r1 = heatramp; // ramp up red
    g1 = 0; // no green
    b1 = 0; // no blue
  }
  
  return strip.Color (r1, g1, b1);
}
