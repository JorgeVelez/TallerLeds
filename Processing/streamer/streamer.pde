import processing.video.*;
import controlP5.*;
import java.io.FilenameFilter;

OPC opc;
PImage dot;

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


int pixelsWidth=54;
int pixelsHeight=28;

String host="192.168.1.79";

Capture cam;

ControlP5 cp5;

int initialBrightness=200;

void setup()
{
  host="127.0.0.1";
  //host="192.168.137.124";
  //host="192.168.1.77";

  //size(270, 165, P3D);
  size(540, 380);
  frameRate(40);
  opc = new OPC(this, host, 7890);
  opc.ledGrid(0, pixelsWidth, pixelsHeight, width/2, height/2-5, 10, 11, 0, true);

  opc.valueToSend=initialBrightness;

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

  cp5.addSlider("Brillo")
    .setPosition(100, 360)
    .setSize(300, 20)
    .setRange(100, 255)
    .setValue(initialBrightness)
    ;
    
      PImage[] imgs = {loadImage("button_a.png"),loadImage("button_b.png"),loadImage("button_c.png")};

      cp5.addButton("next")
     .setPosition(500,360)
     .setImages(imgs)
     .updateSize()
     ;
           PImage[] imgsR = {loadImage("button_aR.png"),loadImage("button_b.png"),loadImage("button_c.png")};

  cp5.addButton("prev")
     .setPosition(470,360)
     .setImages(imgsR)
     .updateSize()
     ;

  dot = loadImage("img/capau letras B.png");

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

void draw()
{
  background(0);

  //if (cam.available() == true) {
  //cam.read();
  // }

  image(movie, 0, 0, width, height);
  //image(cam, 0, 0);

  //float dotSize = width * 0.3;
  //image(dot, mouseX - dotSize/2, mouseY - dotSize/2, dotSize, dotSize);
  //image(dot, 10, 25);
  x = x + thumbSpeed;
  for (int i = 0; i < thumbs.length; i++) {
    image(thumbs[i], - (x + i * thumbSpacing) % thumbStripLength + height , y);
  }

  text(frameRate, 10, 373);
}
