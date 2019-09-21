#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;

#define SERVICE_UUID "0AB8"
#define CHARACTERISTIC_UUID_RX "0AB2"
#define CHARACTERISTIC_UUID_TX "0AB1"

// vvvvvvv LED vvvvvvvvv
#define buttonPin 0   // input pin to use as a digital input
#include "jcbutton.h" // Nice button routine by Jeff Saltzman

#include "EEPROM.h"
#include "FastLED.h"

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

// Fixed definitions cannot change on the fly.
#define LED_DT 15        // Data pin to connect to the strip.
#define LED_CK 11        // Clock pin for the strip.
#define COLOR_ORDER RGB  // Are they RGB, GRB or what??
#define LED_TYPE WS2812B // Don't forget to change LEDS.addLeds
#define NUM_LEDS 36      // Number of LED's.

// Definition for the array of routines to display.
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

//Mode and EEPROM variables
uint8_t maxMode = 9; // Maximum number of display modes. Would prefer to get this another way, but whatever.
int eepaddress = 0;

// Global variables can be changed on the fly.
uint8_t brighntess = 128; // Overall brightness definition. It can be changed on the fly.

struct CRGB leds[NUM_LEDS]; // Initialize our LED array.

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns

typedef void (*SimplePatternList[])(); // List of patterns to cycle through.  Each is defined as a separate function below.

/// config modes
int led_mode = 0;
// blendwawe
CRGB clr1;
CRGB clr2;
uint8_t speed;
uint16_t patternSpeed = 50;
uint16_t colorSpeed = 20;

uint8_t loc1;
uint8_t loc2;
uint8_t ran1;
uint8_t ran2;
// dot_beat// Define variables used by the sequences.
int thisdelay = 10;    // A delay value for the sequence(s)
uint8_t count = 0;     // Count up to 255 and then reverts to 0
uint8_t fadeval = 224; // Trail behind the LED's. Lower => faster fade.
uint8_t bpm = 30;
// animation_a
CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType currentBlending; // NOBLEND or LINEARBLEND
// ======= LED =========

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    // vvvvvvv LED vvvvvvvvv
    Serial.println(101010);
    // ======= LED =========
    if (rxValue == "A")
    {
      gCurrentPatternNumber = (gCurrentPatternNumber + 1) % 9;
    }

    else if (rxValue == "B")
    {
      brighntess = (brighntess + 10) % 128;
      FastLED.setBrightness(brighntess);
    }

    else if (rxValue == "C")
    {
      brighntess = (brighntess - 10) % 128;
      FastLED.setBrightness(brighntess);
    }

    // MODE
    else if (rxValue == "AA")
    {
      gCurrentPatternNumber = 0;
    }
    else if (rxValue == "AB")
    {
      gCurrentPatternNumber = 1;
    }
    else if (rxValue == "AC")
    {
      gCurrentPatternNumber = 2;
    }
    else if (rxValue == "AD")
    {
      gCurrentPatternNumber = 3;
    }
    else if (rxValue == "AE")
    {
      gCurrentPatternNumber = 4;
    }
    else if (rxValue == "AF")
    {
      gCurrentPatternNumber = 5;
    }
    else if (rxValue == "AG")
    {
      gCurrentPatternNumber = 6;
    }
    else if (rxValue == "AH")
    {
      gCurrentPatternNumber = 7;
    }
    else if (rxValue == "AI")
    {
      gCurrentPatternNumber = 8;
    }
    else if (rxValue == "AG")
    {
      gCurrentPatternNumber = 9;
    }

    // BRIGHTNESS
    else if (rxValue == "BA")
    {
      brighntess = 0;
      FastLED.setBrightness(brighntess);
    }
    else if (rxValue == "BB")
    {
      brighntess = 20;
      FastLED.setBrightness(brighntess);
    }
    else if (rxValue == "BC")
    {
      brighntess = 45;
      FastLED.setBrightness(brighntess);
    }
    else if (rxValue == "BD")
    {
      brighntess = 70;
      FastLED.setBrightness(brighntess);
    }
    else if (rxValue == "BE")
    {
      brighntess = 95;
      FastLED.setBrightness(brighntess);
    }
    else if (rxValue == "BF")
    {
      brighntess = 128;
      FastLED.setBrightness(brighntess);
    }

    // SPEED
    else if (rxValue == "CA")
    {
      patternSpeed = 200;
    }
    else if (rxValue == "CB")
    {
      patternSpeed = 160;
    }
    else if (rxValue == "CC")
    {
      patternSpeed = 120;
    }
    else if (rxValue == "CD")
    {
      patternSpeed = 70;
    }
    else if (rxValue == "CE")
    {
      patternSpeed = 30;
    }
    else if (rxValue == "CF")
    {
      patternSpeed = 10;
    }
    else if (rxValue == "DA")
    {
      colorSpeed = 200;
    }
    else if (rxValue == "DB")
    {
      colorSpeed = 160;
    }
    else if (rxValue == "DC")
    {
      colorSpeed = 120;
    }
    else if (rxValue == "DD")
    {
      colorSpeed = 70;
    }
    else if (rxValue == "DE")
    {
      colorSpeed = 40;
    }
    else if (rxValue == "DF")
    {
      colorSpeed = 20;
    }

    txValue++;
  }
};

void setup()
{
  Serial.begin(115200);

  // vvvvvvv LED vvvvvvvvv
  delay(1000); // Soft startup to ease the flow of electrons.

  pinMode(buttonPin, INPUT); // Set button input pin
  digitalWrite(buttonPin, HIGH);

  LEDS.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS); // Use this for WS2812B

  FastLED.setBrightness(brighntess / 2);
  set_max_power_in_volts_and_milliamps(5, 1000); // FastLED power management set at 5V, 500mA.

  gCurrentPatternNumber = EEPROM.read(eepaddress);

  if (gCurrentPatternNumber > maxMode)
    gCurrentPatternNumber = 0; // A safety in case the EEPROM has an illegal value.

  currentBlending = LINEARBLEND;

  // ======= LED =========

  BLEDevice::init("UART Service");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  pServer->getAdvertising()->start();
}

// vvvvvvv LED vvvvvvvvv
SimplePatternList gPatterns = {rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpmx, fill_grad, animation_a, blend_wave, dot_beat};
// ======= LED =========

void loop()
{
  // vvvvvvv LED vvvvvvvvv
  readbutton();

  EVERY_N_MILLIS_I(patternTimer, patternSpeed)
  {
    gPatterns[gCurrentPatternNumber]();
    patternTimer.setPeriod(patternSpeed);
  }

  EVERY_N_MILLIS_I(colorTimer, colorSpeed)
  {
    gHue++;
    colorTimer.setPeriod(colorSpeed);
  }

  EVERY_N_MILLISECONDS(500)
  {
    pTxCharacteristic->setValue(&txValue, 1);
    pTxCharacteristic->notify();
  }
  FastLED.show();
  // ======= LED =========

  // if (deviceConnected)
  // {
  //   pTxCharacteristic->setValue(&txValue, 1);
  //   pTxCharacteristic->notify();
  //   delay(500); // bluetooth stack will go into congestion, if too many packets are sent
  // }
}

void readbutton()
{ // Read the button and increase the mode

  uint8_t b = checkButton();

  if (b == 1)
  { // Just a click event to advance to next pattern
    gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);
    Serial.println(gCurrentPatternNumber);
    Serial.println(patternSpeed);
    Serial.println(colorSpeed);
  }

  if (b == 2)
  { // A double-click event to reset to 0 pattern
    // gCurrentPatternNumber = 0;
    colorSpeed = 50;

    Serial.println(gCurrentPatternNumber);
  }

  if (b == 3)
  { // A hold event to write current pattern to EEPROM
    EEPROM.write(eepaddress, gCurrentPatternNumber);
    Serial.print("Writing: ");
    Serial.println(gCurrentPatternNumber);
  }

} // readbutton()

void CheckMode(int cur_mode)
{
  //TODO: implement read from BLE Write
}

void rainbow()
{

  fill_rainbow(leds, NUM_LEDS, gHue, 7); // FastLED's built-in rainbow generator.

} // rainbow()

void rainbowWithGlitter()
{

  rainbow(); // Built-in FastLED rainbow, plus some random sparkly glitter.
  addGlitter(180);

} // rainbowWithGlitter()

void addGlitter(fract8 chanceOfGlitter)
{

  if (random8() < chanceOfGlitter)
  {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }

} // addGlitter()

void confetti()
{ // Random colored speckles that blink in and fade smoothly.

  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);

} // confetti()

void sinelon()
{ // A colored dot sweeping back and forth, with fading trails.

  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);

} // sinelon()

void bpmx()
{ // Colored stripes pulsing at a defined Beats-Per-Minute.

  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);

  for (int i = 0; i < NUM_LEDS; i++)
  { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }

} // bpmx()

void juggle()
{ // Eight colored dots, weaving in and out of sync with each other.

  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;

  for (int i = 0; i < 8; i++)
  {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }

} // juggle()

void fill_grad()
{

  uint8_t starthue = beatsin8(5, 0, 255);
  uint8_t endhue = beatsin8(7, 0, 255);

  if (starthue < endhue)
  {
    fill_gradient(leds, NUM_LEDS, CHSV(starthue, 255, 255), CHSV(endhue, 255, 255), FORWARD_HUES); // If we don't have this, the colour fill will flip around.
  }
  else
  {
    fill_gradient(leds, NUM_LEDS, CHSV(starthue, 255, 255), CHSV(endhue, 255, 255), BACKWARD_HUES);
  }

} // fill_grad()

void animation_a()
{ // running red stripe.
  for (int i = 0; i < NUM_LEDS; i++)
  {
    uint8_t red = (millis() / 5) + (i * 12); // speed, length
    if (red > 128)
    {
      red = 0;
      leds[i] = ColorFromPalette(currentPalette, red, red, currentBlending);
    }
  }
} // animation_a()

void blend_wave()
{
  speed = beatsin8(6, 0, 255);

  clr1 = blend(CHSV(beatsin8(3, 0, 255), 255, 255), CHSV(beatsin8(4, 0, 255), 255, 255), speed);
  clr2 = blend(CHSV(beatsin8(4, 0, 255), 255, 255), CHSV(beatsin8(3, 0, 255), 255, 255), speed);

  loc1 = beatsin8(10, 0, NUM_LEDS - 1);

  fill_gradient_RGB(leds, 0, clr2, loc1, clr1);
  fill_gradient_RGB(leds, loc1, clr2, NUM_LEDS - 1, clr1);
} // blend_wave()

void dot_beat()
{

  uint8_t inner = beatsin8(bpm, NUM_LEDS / 4, NUM_LEDS / 4 * 3);  // Move 1/4 to 3/4
  uint8_t outer = beatsin8(bpm, 0, NUM_LEDS - 1);                 // Move entire length
  uint8_t middle = beatsin8(bpm, NUM_LEDS / 3, NUM_LEDS / 3 * 2); // Move 1/3 to 2/3

  leds[middle] = CRGB::Purple;
  leds[inner] = CRGB::Blue;
  leds[outer] = CRGB::Aqua;

  nscale8(leds, NUM_LEDS, fadeval); // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);

} // dot_beat()
