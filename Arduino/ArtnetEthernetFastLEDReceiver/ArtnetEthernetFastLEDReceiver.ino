/*
   x universos a 30fps
*/

#include <ArtnetWifi.h>
#include <Arduino.h>
#include <FastLED.h>
#include <ETH.h>

static bool eth_connected = false;
static bool artnet_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      initialize();

      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}
// Wifi settings
// LED settings
const int clockPin = 4;//D5 esp32
//CRGB leds[numLeds];
// Art-Net settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
const char host[] = "192.168.1.4";

//timer
unsigned long DELAY_TIME = 0; // 10 sec
unsigned long INTERVAL = 1000;

int packetCounter = 0;
int frameCounter = 0;

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

int ledCounter = 0;
int individualLedCounter = 0;
int channelCounter = 0;
int channelGlobalCounter = 0;

uint8_t RedB;
uint8_t GreenB;
uint8_t BlueB;

int brightness = 200;

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
  universesReceived[universe] = 1;

  if (universe == 0)
  {
    frameCounter++;

    //Serial.print("length:");
    //Serial.print (length);
    //Serial.print(" first:");
    //Serial.println (data[0]);

    FastLED.show();
  }

  individualLedCounter = 0;
  int compensator = 0;
  for (int i = 0; i < length ; i++)
  {
    if (universe == 0) {
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = ((universe * 512) + (i) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 1) {
      compensator = 1;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 2) {
      compensator = 2;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 3) {
      compensator = 0;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 4) {
      compensator = 1;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 5) {
      compensator = 2;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 6) {
      compensator = -1;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 7) {
      compensator = 1;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 8) {
      compensator = -1;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 9) {
      compensator = 0;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }

    if (universe == 10) {
      compensator = -2;
      if (individualLedCounter == 0) {
        individualLedCounter++;
      } else  if (individualLedCounter == 1) {
        individualLedCounter++;
      } else  if (individualLedCounter == 2) {
        int ledaqui = floor(((universe * 512) + (i)) / 3);
        if (ledaqui < NUM_LEDS)
          leds[ledaqui] = CRGB(data[i - 2 + compensator ], data[i - 1 + compensator ], data[i + compensator ]);
        individualLedCounter = 0;
      }
    }



  }

  //5184
  //5632
  packetCounter++;


  if (universe == maxUniverses)
  {
    if (brightness != data[256 ]) {
      brightness = data[256 ];
      FastLED.setBrightness(data[256 ] );
      // Serial.print("brightness changed: ");
      // Serial.println(brightness);

    }
    sendFrame = 1;
    for (int i = 0 ; i < maxUniverses ; i++)
    {
      if (universesReceived[i] == 0)
      {
        //Serial.println("Broke");
        sendFrame = 0;
        break;
      }
    }
    channelCounter = 0;
    channelGlobalCounter = 0;
    ledCounter = 0;
    individualLedCounter = 0;
    if (sendFrame) {
    } else {
      //Serial.println("lost packet");
    }
    //memset(leds, 0, numberOfChannels);
    memset(universesReceived, 0, maxUniverses);

  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  ETH.config(IPAddress(192, 168, 1, 44),IPAddress(192, 168, 1, 1),IPAddress(255, 255, 255, 0),IPAddress(192, 168, 1, 1),IPAddress(192, 168, 1, 1));

}

void initialize()
{
  Serial.print("initializing:");
  artnet.begin(host);
  artnet.setLength(3);
  artnet.setUniverse(startUniverse);

  FastLED.addLeds<APA102, 2, clockPin , BGR>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 14, clockPin, BGR>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 15, clockPin, BGR>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 12, clockPin, BGR>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 13, clockPin, BGR>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 32, clockPin, BGR>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, 33, clockPin, BGR>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  //FastLED.addLeds<APA102, 39, clockPin, BGR>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);


  FastLED.setBrightness(brightness);

  initTest();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  Serial.print("universes:");
  Serial.println(maxUniverses);
  artnet_connected = true;
}

void loop()
{
  if (artnet_connected) {
    artnet.read();
  }
  if (millis()  >= (DELAY_TIME + INTERVAL) && eth_connected) {
    DELAY_TIME = millis(); // finished delay -- single shot, once only
    Serial.print("frames: ");
    Serial.print(frameCounter);
    Serial.print("packets: ");
    Serial.println(packetCounter);
    frameCounter = 0;
    packetCounter = 0;

    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());

    uint8_t val=222;
    artnet.setByte(0, val);
    artnet.setByte(1, val++);
    artnet.setByte(2, val++);
    artnet.write();
  }

}
