#include <ArtnetWifi.h>
#include <Arduino.h>
#include <AsyncTCP.h>
#include <FastLED.h>
#include <ETH.h>
#include <ESPAsyncWebServer.h>
#include <ESPDMX.h>
#include <SPIFFS.h>
#include "EEPROM.h"

DMXESPSerial dmx;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//////////////////////////// webserver
#define isDebug false

const char* PARAM_CALIBRA = "calibra";
const char* PARAM_VELMAS = "velmas";

int state;
const int CalibrateStart = 1;
const int LookingForZero = 2;
const int LookingForEnd = 3;
const int BuscandoHome = 4;
const int Moviendo = 5;
const int Rutina = 6;

const int Iddle = -1;
int velocidad = 6;



/////////////////////////////EEPROM////////////////////////////
int addressVelocity = 0;
#define EEPROM_SIZE 64

//////////////////////////////////////////////////////
bool ledState = 0;
const int ledPin = 2;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Controlador ART-NET</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
  html {
    font-family: Arial, Helvetica, sans-serif;
    text-align: center;
  }
  h1 {
    font-size: 1.8rem;
    color: white;
  }
  h2{
    font-size: 1.5rem;
    font-weight: bold;
    color: #143642;
  }
  .topnav {
     background-color: #000000;
  }
  body {
    margin: 0;
    background-color: #000000;
  }
  .content {
    padding: 30px;
    max-width: 600px;
    margin: 0 auto;
  }
  .card {
    background-color: #F8F7F9;;
    box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
    padding-top:10px;
    padding-bottom:20px;
  }
  .buttonC {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #FC5C65;
    border: none;
    border-radius: 5px;
   }
     .buttonA {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #A65EEA;
    border: none;
    border-radius: 5px;
   }
     .buttonP {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #FF0000;
    border: none;
    border-radius: 5px;
   }
     .buttonS {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #F7B731;
    border: none;
    border-radius: 5px;
   }

    .buttonV {
    padding: 15px 50px;
    font-size: 24px;
    text-align: center;
    outline: none;
    color: #fff;
    background-color: #10B9B1;
    border: none;
    border-radius: 5px;
   }
   /*.button:hover {background-color: #0f8b8d}
   .buttonC:active {
     background-color: #FC5C65;
     box-shadow: 2 2px #CDCDCD;
     transform: translateY(2px);
   }*/
   .state {
     font-size: 1.5rem;
     color:#8c8c8c;
     font-weight: bold;
   }
  </style>
<title>Controlador Art-Net</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="icon" href="data:,">
</head>
<body>
  <div class="topnav">
    <img src="logo">
  </div>
  <div class="content">
    <div>
      <p class="state" style="display:none" id="textoCalibrando">Calibrando...</p>
      <p class="state"  id="textoVel">Velocidad: <span  id="state">%VEL%s</span></p>
      <p><button  id="buttonVmas" class="buttonV">+</button></p>
      <p><button id="buttonC" class="buttonC">CALIBRAR</button></p>

    </div>
  </div>
<script>
 
var PARAM_CALIBRA = "calibra";
var PARAM_VELMAS = "velmas";

 var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  window.addEventListener('load', onLoad);///////////////////////////
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; 
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  function onMessage(event) {
    var state;
    if (event.data == "calibrado"){
    document.getElementById('buttonVmas').style.display = 'inline';
    document.getElementById('textoVel').style.display = 'inline';
     document.getElementById('textoCalibrando').style.display = 'none';
     document.getElementById('buttonC').style.display = 'inline';
    }
    else{
      document.getElementById('state').innerHTML = event.data+"s";
    }
  }
  
  function onLoad(event) {
    initButtonS();
    initWebSocket();
  }
  function initButtonS() {
    document.getElementById('buttonC').addEventListener('click', () => { buttonHandler(PARAM_CALIBRA) });
    document.getElementById('buttonVmas').addEventListener('click', () => { buttonHandler(PARAM_VELMAS) });
  }
  
  function buttonHandler(msg){
    websocket.send(msg);
    if(msg==PARAM_CALIBRA){
      document.getElementById('buttonC').style.display = 'none';
      document.getElementById('textoCalibrando').style.display = 'inline';
    }
    console.log('msg:'+msg);
  }
</script>
</body>
</html>
)rawliteral";

void notifyClients(String msg) {
  ws.textAll(msg);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
  if (strcmp((char*)data, PARAM_CALIBRA) == 0) {    
      state = CalibrateStart;
        if(isDebug){
      delay(2000);
      notifyClients("calibrado");
        }
      Serial.println("command PARAM_CALIBRA ");
      
    }else if (strcmp((char*)data, PARAM_VELMAS) == 0) {    
      velocidad++;
      if(velocidad>15)
      velocidad=15;
      EEPROM.writeInt(addressVelocity, velocidad); 
      EEPROM.commit();
       delay(100);
      notifyClients(String(velocidad));
      Serial.println("command PARAM_VELMAS ");
      Serial.println(velocidad);
    }else {
      Serial.println("command unknown ");
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
    Serial.printf("WebSocket client #%u data\n", data);
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.print("processor "); 
  Serial.println(var);
  if(var == "VEL"){
    return String(velocidad);
  }
  return String();
}

uint32_t interval = 1000/30;
uint32_t now=0;

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

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
    Serial.println("ETH SYSTEM_EVENT_ETH_GOT_IP");
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
const int clockPin = 4;//D5 esp32
ArtnetWifi artnet;

//timer
unsigned long DELAY_TIME = 0; // 10 sec
unsigned long INTERVAL = 1000;

int packetCounter = 0;
int frameCounter = 0;

// Check if we got all universes
const int maxUniverses = 11;
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

#define NUM_STRIPS 8
#define NUM_LEDS_PER_STRIP 180
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
  int counterDMX = 1;
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

    if (universe == 11) {
      dmx.write(i,  data[i ]);
      
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
  ETH.config(IPAddress(192, 168, 1, 100  ),IPAddress(192, 168, 1, 1),IPAddress(255, 255, 255, 0),IPAddress(192, 168, 1, 1),IPAddress(192, 168, 1, 1));

}

void initialize()
{
  Serial.print("initializing:");
  artnet.begin();
//  artnet.begin(host);
//  artnet.setLength(3);
//  artnet.setUniverse(healthUniverse);

  //FastLED.addLeds<APA102, 2, clockPin , BGR>(leds, 0, NUM_LEDS_PER_STRIP);
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


if (!EEPROM.begin(1000)) {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
   velocidad=EEPROM.readInt(addressVelocity);
 Serial.print("La velocidad guardada es: ");
  Serial.println(velocidad);

  if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        //return;
  }

  initWebSocket();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/logo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.png", "image/png");
  });  

  server.onNotFound(notFound);

  server.begin();

  dmx.init(512, 2);  

  //se inicializa 
  //fastled
  //artnet, ya esta esperando udp
  //eeprom, ya puede leer cosas guardadas
  //websocket, ya espera mensajes y eventos
  //respuesta server, en cuanto hay un request, loadea la pag, inicializa websocket local
  //empieza server, para esperar requests
  //dmx, ya esta mandando y espera dmx para cambiar sus valores
}

void loop()
{
    ws.cleanupClients();

  if (artnet_connected) {
    artnet.read();
  }
  
  if (millis()  >= (DELAY_TIME + INTERVAL) && eth_connected) {
    DELAY_TIME = millis(); // finished delay -- single shot, once only
//    Serial.print("frames: ");
//    Serial.print(frameCounter);
//    Serial.print("packets: ");
//    Serial.println(packetCounter);
//    
//
//    Serial.print("ETH MAC: ");
//    Serial.print(ETH.macAddress());
//    Serial.print(", IPv4: ");
//    Serial.print(ETH.localIP());

notifyClients(String(frameCounter));
    frameCounter = 0;
    packetCounter = 0;    
  }

//  if (millis() - now >= interval)
//  {
//    now=millis();
//    dmx.update();
//  }
}
