import themidibus.*; //Import the library
import ch.bildspur.artnet.*;
import processing.serial.*;

Serial myPort;  // Create object from Serial class
int val;   
ArtNetClient artnet;
byte[] dmxData = new byte[512];
MidiBus myBus; // The MidiBus

  float averageValue =0;
  color circleColor;
  String ip="192.168.1.125";
void setup() {
  size(400, 400);
  background(0);
  
  //frameRate(200);
  
  colorMode(HSB, 360, 100, 100);
  textAlign(CENTER, CENTER);
  textSize(20);

  MidiBus.list(); // List all available Midi devices on STDOUT. This will show each device's index and name.
  myBus = new MidiBus(this, 0, 3); // Create a new MidiBus with no input device and the default Java Sound Synthesizer as the output device.

  // create artnet client without buffer (no receving needed)
  artnet = new ArtNetClient(null);
  artnet.start();
  
  //String portName = Serial.list()[0];
   //println(Serial.list());
  //myPort = new Serial(this, portName, 9600);
  
   for (int i = 0; i < dmxData.length; i++) {
    dmxData[i] = byte(random(128)); 
  }
}

void draw() {
  //println("Sending");
  //int channel = 0;
  //int pitch = 64;
  //int velocity = 127;

  //myBus.sendNoteOn(channel, pitch, velocity); // Send a Midi noteOn
  //delay(200);
  //myBus.sendNoteOff(channel, pitch, velocity); // Send a Midi nodeOff

  //int number = 0;
  //int value = 90;

  //myBus.sendControllerChange(channel, number, value); // Send a controllerChange
  //delay(2000);
  
  //if ( myPort.available() > 0) {  // If data is available,
  //  val = myPort.read();         // read it and store it in val
  //    println("val:"+val);

  //}
  
   background(0,255, 0, 5);
  

  
  // Map the mean value onto the radius of the circle
  float circleRadius = map(averageValue, 0, 1.0, 0, 40);
  
  
  circleColor = color(0, 408, 612, 816);
  
  // create a loop between the colors
  if (averageValue > 10) {
    //circleColor = lerpColor(verde, blu, averageValue);
  }
  if (averageValue > 40) {
    //circleColor = lerpColor(blu, rosso, averageValue);
  }
  
  
  noStroke();
  fill(circleColor);

  //ellipse(random(windowWidth), random(windowHeight), circleRadius, circleRadius);
  ellipse(20, 20, circleRadius, circleRadius);
  
  fill(255);
  textSize(15);
  text(ip, 60, 40); 
  
averageValue -=.1;
if(averageValue<0)
averageValue=0;

if (mousePressed == true) {
    
  artnet.unicastDmx("192.168.1.9", 0, 0, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 1, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 2, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 4, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 4, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 6, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 7, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 8, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 9, dmxData);
  artnet.unicastDmx("192.168.1.9", 0, 10, dmxData);
  }
}

void noteOn(int channel, int pitch, int velocity) {
  // Receive a noteOn
  println();
  println("Note On:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
  
   // create color
  int c = color(frameCount % 360, 80, 100);

  background(c);

  // fill dmx array
  dmxData[0] = (byte) 1;
  dmxData[1] = (byte) pitch;
  dmxData[2] = (byte) velocity;

  // send dmx to localhost
artnet.unicastDmx("192.168.1.125", 0, 0, dmxData);
artnet.unicastDmx("192.168.1.125", 0, 0, dmxData);
artnet.unicastDmx("192.168.1.125", 0, 0, dmxData);
artnet.unicastDmx("192.168.1.125", 0, 0, dmxData);
artnet.unicastDmx("192.168.1.125", 0, 0, dmxData);
artnet.unicastDmx("192.168.1.125", 0, 0, dmxData);
  // show values
  text("R: " + (int)red(c) + " Green: " + (int)green(c) + " Blue: " + (int)blue(c), width / 2, height / 2);

//myPort.write((byte) 1); 
//myPort.write((byte) pitch); 
//myPort.write((byte) velocity); 
//myPort.write('\n'); 
//myPort.write((byte) velocity); 
averageValue =1;
}

void noteOff(int channel, int pitch, int velocity) {
  // Receive a noteOff
  println();
  println("Note Off:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
  
    // create color
  int c = color(frameCount % 360, 80, 100);

  background(c);

  // fill dmx array
  dmxData[0] = (byte) 0;
  dmxData[1] = (byte) pitch;
  dmxData[2] = (byte) velocity;

  // send dmx to localhost
  //artnet.unicastDmx("127.0.0.1", 0, 0, dmxData);
  artnet.unicastDmx(ip, 0, 0, dmxData);
  // show values
  text("R: " + (int)red(c) + " Green: " + (int)green(c) + " Blue: " + (int)blue(c), width / 2, height / 2);

//myPort.write((byte) 0); 
//myPort.write((byte) pitch); 
//myPort.write((byte) velocity); 
//myPort.write('\n');  
}

void controllerChange(int channel, int number, int value) {
  // Receive a controllerChange
  println();
  println("Controller Change:");
  println("--------");
  println("Channel:"+channel);
  println("Number:"+number);
  println("Value:"+value);
}

void delay(int time) {
  int current = millis();
  while (millis () < current+time) Thread.yield();
}
