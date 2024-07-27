/*
This example will receive multiple universes via Art-Net and control a strip of
WS2812 LEDs via the FastLED library: https://github.com/FastLED/FastLED
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/
#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>

// Wifi settings
const char* ssid = "ChosgangT";
const char* password = "sonia2020";

// LED settings
const int clockPin = 12;//D5 esp32
const byte dataPin = 4;//D19 esp32
//CRGB leds[numLeds];
// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = 10;
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

#define NUM_STRIPS 8
#define NUM_LEDS_PER_STRIP 216
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS

CRGB leds[NUM_LEDS];
const int numberOfChannels = NUM_LEDS * 3; // Total number of channels you want to receive (1 led = 3 channels)

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

void initTest()
{
   Serial.println("");
  Serial.println("Testing red1");
  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
    Serial.println("Testing green");

  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
      Serial.println("Testing blue");

  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
      Serial.println("start listen ing");

  for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{

      Serial.println("onDmxFrame");

  if(data[0]==0){
     for (int i = 0 ; i < 80 ; i++) {
    leds[i] =  CRGB(0, 0, 0);
  }
  }else{
     for (int i = 0 ; i < 80 ; i++) {
      
    leds[i] =  CHSV((data[1]-50)*8, 255, 255); 
  }
  }
    FastLED.show();
    Serial.println("onDmxFrame");


    // Reset universeReceived to 0
    //memset(universesReceived, 0, maxUniverses);
  


  // read universe and put into the right part of the display buffer
//  for (int i = 0; i < length / 3; i++)
//  {
//    int led = i ;//+ (universe*170);
//    if (led < NUM_LEDS)
//      leds[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//  }
//  previousDataLength = length;
//
//  if (universe==maxUniverses)
//  {
//    FastLED.show();
//    // Reset universeReceived to 0
//    memset(universesReceived, 0, maxUniverses);
//  }
}

void setup()
{
  Serial.begin(115200);
  bool connectedIP=ConnectWifi();
  artnet.begin();
   FastLED.addLeds<APA102, 4,clockPin , BGR>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 21, clockPin, BGR>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 22, clockPin, BGR>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 23, clockPin, BGR>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 18, clockPin, BGR>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 5, clockPin, BGR>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 16, clockPin, BGR>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 19, clockPin, BGR>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);

FastLED.setBrightness(100);
if(connectedIP)
  initTest();
  else{
    for (int i = 0 ; i < NUM_LEDS ; i++) {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  }

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
Serial.print("universes:");
  Serial.println(maxUniverses);
  
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();

//   if (Serial.available() > 0)
//    {
//        int onoff = Serial.readStringUntil(';').toInt();
//        int color = Serial.readStringUntil('\n').toInt();
//for (int i = 0 ; i < 80 ; i++) {
//      
//    leds[i] =  CHSV(0, 255, 255); 
//  }
//
//
//          if(onoff==0){
//     for (int i = 0 ; i < 80 ; i++) {
//    leds[i] =  CRGB(0, 0, 0);
//  }
//  }else{
//     for (int i = 0 ; i < 80 ; i++) {
//      
//    leds[i] =  CHSV((color-50)*8, 255, 255); 
//  }
//  }
//    FastLED.show();
//    }
}
