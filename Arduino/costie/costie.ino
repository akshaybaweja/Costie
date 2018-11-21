#include <Adafruit_NeoPixel.h>
//Include files for MX90615 Temperature Sensor
#include "MLX90615.h"
#include <I2cMaster.h>

#define PIN 3
#define STRIPSIZE 20

#define SDA_PIN A4   //define the SDA pin
#define SCL_PIN A5   //define the SCL pin

const int threshold = 3;

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel ring = Adafruit_NeoPixel(STRIPSIZE, PIN, NEO_GRB);

SoftI2cMaster i2c(SDA_PIN, SCL_PIN);
MLX90615 mlx90615(DEVICE_ADDR, &i2c);

float objectTemp = 0.0, ambientTemp = 0.0;

void setup() {
  Serial.begin(9600);
  ring.begin();
  ring.setBrightness(25);  // Lower brightness and save eyeballs!
  ring.show(); // Initialize all pixels to 'off'
}

void showTemp() {
  Serial.print("Object temperature: ");
  objectTemp = mlx90615.getTemperature(MLX90615_OBJECT_TEMPERATURE);
  Serial.println(objectTemp);
  Serial.print("Ambient temperature: ");
  ambientTemp = mlx90615.getTemperature(MLX90615_AMBIENT_TEMPERATURE);
  Serial.println(ambientTemp);
}

void loop() {

  showTemp();

  float difference = objectTemp - ambientTemp;
  Serial.print("Differnce: ");
  Serial.println(difference);

  if (difference >= threshold) {
    colorWave(50, 'r');
  } else if (difference <= -1*threshold) {
    colorWave(50, 'b');
  } else {
//    colorWipe(ring.Color(0, 0, 0), 10); // Black
    rainbow(20);
  }
  delay(100);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < ring.numPixels(); i++) {
    ring.setPixelColor(i, c);
    ring.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < ring.numPixels(); i++) {
      ring.setPixelColor(i, Wheel((i + j) & 255));
    }
    ring.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < ring.numPixels(); i++) {
      ring.setPixelColor(i, Wheel(((i * 256 / ring.numPixels()) + j) & 255));
    }
    ring.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return ring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return ring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return ring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

/**
        ^   ^   ^
   ~~~~~ ColorWave ~~~~~
          V   V   V
*/
void colorWave(uint8_t wait, char type) {
  int i, j, stripsize, cycle;
  float ang, rsin, gsin, bsin, offset;

  static int tick = 0;

  stripsize = ring.numPixels();
  cycle = stripsize; // times around the circle...

  while (++tick % cycle) {
    offset = map2PI(tick);

    for (i = 0; i < stripsize; i++) {
      ang = map2PI(i) - offset;
      rsin = sin(ang);
      //            gsin = sin(2.0 * ang / 3.0 + map2PI(int(stripsize / 6)));
      //            bsin = sin(4.0 * ang / 5.0 + map2PI(int(stripsize / 3)));
      switch (type) {
        case 'r': ring.setPixelColor(i, ring.Color(trigScale(rsin), 0, 0));
          break;
        case 'g': ring.setPixelColor(i, ring.Color(0, trigScale(rsin), 0));
          break;
        case 'b': ring.setPixelColor(i, ring.Color(0, 0, trigScale(rsin)));
          break;
        default: ring.setPixelColor(i, ring.Color(trigScale(rsin), trigScale(gsin), trigScale(bsin)));
      }
    }

    ring.show();
    delay(wait);
  }

}

/**
   Scale a value returned from a trig function to a byte value.
   [-1, +1] -> [0, 254]
   Note that we ignore the possible value of 255, for efficiency,
   and because nobody will be able to differentiate between the
   brightness levels of 254 and 255.
*/
byte trigScale(float val) {
  val += 1.0; // move range to [0.0, 2.0]
  val *= 127.0; // move range to [0.0, 254.0]

  return int(val) & 255;
}

/**
   Map an integer so that [0, striplength] -> [0, 2PI]
*/
float map2PI(int i) {
  return PI * 2.0 * float(i) / float(ring.numPixels());
}
