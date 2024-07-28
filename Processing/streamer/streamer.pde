import processing.video.*;
import controlP5.*;
import themidibus.*;
import java.io.FilenameFilter;
import java.net.InetAddress;
import java.net.UnknownHostException;

OPC opc;
PImage dot;
MidiBus myBus;
//////////////////IMAGES

PImage[] thumbs;
float x;
float y;
float thumbSpacing;
float thumbStripLength;
float thumbSpeed;

String[] filenames;

//////////////////MOVIES
Movie[] movies;
Movie movie;
String[] movieFilenames;

int movieIndex=0;


int pixelsWidth=60;
int pixelsHeight=28;

String host="192.168.1.44";
String localInterface="192.168.1.4";

Capture cam;

ControlP5 cp5;

int initialBrightness=200;

color tintColor=#FFFFFF;

void setup()
{ 
  //size(270, 165, P3D);
  size(540, 380);
  frameRate(55);
  opc = new OPC(this, host, localInterface, 7890);
  opc.ledGrid(0, pixelsWidth, pixelsHeight, width/2, height/2-5, 9, 11, 0, true);

  opc.valueToSend=initialBrightness;
  
  ///////////////////////MIDI
   MidiBus.list(); 
   myBus = new MidiBus(this, "loopMIDI Port", 0);

  ///////////////////////MOVIES

  movieFilenames = loadFilenames(dataPath("mov"), ".mp4");
  println(movieFilenames);
  movies = new Movie[movieFilenames.length];
  movie = new Movie(this, dataPath("mov")+"/"+movieFilenames[movieIndex]);
  movie.loop();

  for (int i = 0; i < movies.length; i++) {
   // movies[i] = new Movie(this, dataPath("mov")+"/"+movieFilenames[i]);
  //movies[i].loop();
  }
  
  ///////////////////////IMAGES

  filenames = loadFilenames(dataPath("img"), ".png");
  //println(filenames);
  thumbs = new PImage[filenames.length];

  for (int i = 0; i < thumbs.length; i++) {
    thumbs[i] = loadImage(dataPath("img")+"/"+filenames[i]);
  }
  y=10;
  x=100;
  thumbSpacing = 1600;
  thumbStripLength = thumbSpacing*thumbs.length;
  thumbSpeed = 0.5;

  //////////////////////   //UI
  cp5 = new ControlP5(this);

  //cp5.addSlider("Brillo")
  //  .setPosition(100, 360)
  //  .setSize(300, 20)
  //  .setRange(100, 255)
  //  .setValue(initialBrightness)
  //  ;
    
  //    PImage[] imgs = {loadImage("button_a.png"),loadImage("button_a.png"),loadImage("button_a.png")};

  //    cp5.addButton("next")
  //   .setPosition(500,360)
  //   .setImages(imgs)
  //   .updateSize()
  //   ;
  //         PImage[] imgsR = {loadImage("button_aR.png"),loadImage("button_aR.png"),loadImage("button_aR.png")};

  //cp5.addButton("prev")
  //   .setPosition(470,360)
  //   .setImages(imgsR)
  //   .updateSize()
  //   ;

  dot = loadImage("color-dot.png");

  /*String[] cameras = Capture.list();
   
   if (cameras.length == 0) {
   println("There are no cameras available for capture.");
   //exit
   cam = new Capture(this, "pipeline:autovideosrc");
   cam.start();
   } else {
   println("Available cameras:");
   for (int i = 0; i < cameras.length; i++) {
   println(cameras[i]);
   }
   cam = new Capture(this, cameras[0]);
   cam.start();
   }
   */
}

void movieEvent(Movie m) {
  m.read();
}

public void next(int theValue) {
movieIndex++;
  if(movieIndex>movies.length-1)
  movieIndex=0;
  
    movie = new Movie(this, dataPath("mov")+"/"+movieFilenames[movieIndex]);

}

public void prev(int theValue) {
  movieIndex--;
  if(movieIndex<0)
  movieIndex=movies.length;
  
    movie = new Movie(this, dataPath("mov")+"/"+movieFilenames[movieIndex]);

}

void Brillo(int theColor) {
  opc.valueToSend=theColor;
}

String[] loadFilenames(String path, String ext) {
  File folder = new File(path);
  FilenameFilter filenameFilter = new FilenameFilter() {
    public boolean accept(File dir, String name) {
      return name.toLowerCase().endsWith("mp4"); // change this to any extension you want
    }
  };
  return folder.list(filenameFilter);
}


void noteOn(int channel, int pitch, int velocity) {
  // Receive a noteOn
  println();
  println("Note On:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
  
  colorMode(HSB, 255);
  tintColor = color(pitch, 255, 255);
}

void noteOff(int channel, int pitch, int velocity) {
  // Receive a noteOff
  println();
  println("Note Off:");
  println("--------");
  println("Channel:"+channel);
  println("Pitch:"+pitch);
  println("Velocity:"+velocity);
  
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


void draw()
{
  background(0);

  //if (cam.available() == true) {
  //cam.read();
  // }

  tint(tintColor);
  image(movie, 0, 0, width, height);
  //image(cam, 0, 0);
  
  x = x + thumbSpeed;
  for (int i = 0; i < thumbs.length; i++) {
    image(thumbs[i], - (x + i * thumbSpacing) % thumbStripLength + height , y);
  }

  text("RFPS:"+opc.remoteFPS, 10, 347);
  text("RPCKS:"+opc.remotePackets, 10, 360);
  text((int)frameRate, 10, 373);
  
  fill(255);
  circle( mouseX, mouseY , 15);
}
