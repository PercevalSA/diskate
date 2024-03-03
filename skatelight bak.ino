// discovid.ino
// luminous skateboard and interactive art

// LEDs 
#include "FastLED.h"                                          // FastLED library.

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define LED_DT 11                                             // Serial data pin for WS2812 or WS2801.
#define LED_CK 13                                             // Serial clock pin for WS2801 or APA102.
#define COLOR_ORDER BGR                                       // Are they GRB for WS2812 and GBR for APA102
#define LED_TYPE APA102                                       // What kind of strip are you using? WS2812, APA102. . .
#define NUM_LEDS 26                                           // Number of LED's.

#define SPI_SPEED DATA_RATE_MHZ(12)

// switchs for interraction
#define SWITCH_1 2
#define SWITCH_2 3
#define SWITCH_3 4
#define SWITCH_4 5

// LEDs data
uint8_t brightness = 255;

CRGB leds[NUM_LEDS];

uint8_t maxChanges = 24;      // Value for blending between palettes.

CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;                                // NOBLEND or LINEARBLEND 

uint16_t Xorig = 0x012;
uint16_t Yorig = 0x015;
uint16_t X;
uint16_t Y;
uint16_t Xn;
uint16_t Yn;
uint8_t index;

/* switch data
 * we store a 4 bits number with the 4 switches 
 *
 */
int switch1State = 0;
int switch2State = 0;
int switch3State = 0;
int switch4State = 0;

void setup() {
    Serial.begin(115200);                                        // Initialize serial port for debugging.
    delay(1000);                                                // Soft startup to ease the flow of electrons.

    // initialize switchs
    pinMode(SWITCH_1, INPUT);
    pinMode(SWITCH_2, INPUT);
    pinMode(SWITCH_3, INPUT);
    pinMode(SWITCH_4, INPUT);

    // initialize LEDs
    FastLED.addLeds<LED_TYPE, LED_DT, LED_CK, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(brightness);
    currentBlending = LINEARBLEND;

    X=Xorig;
    Y=Yorig;
}

// TODO faire des transition entre les états au changement des boutton
// ne pas le faire brutallement, attendre la fin d'un cycle
void loop() {

    meteorRain(0xff,0xff,0xff,10, 64, true, 30);
/*    if(switch1State) {
        serendipitous_main();
    } else {
        blur();
    }
*/
    LEDS.show();
    
    EVERY_N_MILLISECONDS(1000) {
        // read the state of the switchs
        switch1State = digitalRead(SWITCH_1);
        switch2State = digitalRead(SWITCH_2);
        switch3State = digitalRead(SWITCH_3);
        switch4State = digitalRead(SWITCH_4);

        // check if the pushswitch is pressed. If it is, the switchState is HIGH:
        if (switch1State == HIGH) {
            Serial.println("Boutton 1 : En bas");
        } else {
            Serial.println("Boutton 1 : En haut");
        }
    }
}


inline void serendipitous_main() {
    EVERY_N_MILLISECONDS(60) {
        nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // Blend towards the target palette
    }

    EVERY_N_MILLISECONDS(50) {
        serendipitous();     
    }
}

/* à reprendre avec le papier officiel
 *  This is from the Serendipitous Circles article in the August 1977 and April 1978 issues of Byte Magazine. 
 *  This is a poorly rendered 1D version and I really should get around to converting it to 2D and buying a 32x32 matrix to display it. Some day . . 
 *  
 *  Check the magazine article out online, as this 1D routine doesn't do it justice.
 *
 */
void serendipitous() {

    EVERY_N_SECONDS(5) {

        uint8_t baseC = random8();
        targetPalette = CRGBPalette16(CHSV(baseC-3, 255, random8(192,255)), CHSV(baseC+2, 255, random8(192,255)), CHSV(baseC+5, 192, random8(192,255)), CHSV(random8(), 255, random8(192,255)));

        X = Xorig;
        Y = Yorig;    
    }

    Xn = X-(Y/3); Yn = Y+(X/1.5);

    X = Xn;
    Y = Yn;

    index=(sin8(X)+cos8(Y))/2;                            // Guarantees maximum value of 255
    CRGB newcolor = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);

    //  nblend(leds[X%NUM_LEDS-1], newcolor, 224);          // Try and smooth it out a bit. Higher # means less smoothing.
    nblend(leds[map(X,0,65535,0,NUM_LEDS)], newcolor, 224); // Try and smooth it out a bit. Higher # means less smoothing.
    fadeToBlackBy(leds, NUM_LEDS, 32);                      // 8 bit, 1 = slow, 255 = fast

}

void blur() {

    uint8_t blurAmount = dim8_raw( beatsin8( 3, 64, 192) );      // A sinewave at 3 Hz with values ranging from 64 to 192.
    blur1d( leds, NUM_LEDS, blurAmount);                         // Apply some blurring to whatever's already on the strip, which will eventually go black.

    uint8_t  i = beatsin8( 9, 0, NUM_LEDS);
    uint8_t  j = beatsin8( 7, 0, NUM_LEDS);
    uint8_t  k = beatsin8( 5, 0, NUM_LEDS);

    // The color of each point shifts over time, each at a different speed.
    uint16_t ms = millis();  
    leds[(i+j)/2] = CHSV( ms / 29, 200, 255);
    leds[(j+k)/2] = CHSV( ms / 41, 200, 255);
    leds[(k+i)/2] = CHSV( ms / 73, 200, 255);
    leds[(k+i+j)/3] = CHSV( ms / 53, 200, 255);

}

void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  FastLED.clear();

  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {

    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        leds[j].fadeToBlackBy( meteorTrailDecay );      
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        FastLED.setPixel(i-j, red, green, blue);
      } 
    }
   
    showStrip();
    delay(SpeedDelay);
  }
}
