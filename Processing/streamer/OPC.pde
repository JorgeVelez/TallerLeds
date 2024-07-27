/*
 * Simple Open Pixel Control client for Processing,
 * designed to sample each LED's color from some point on the canvas.
 *
 * Micah Elizabeth Scott, 2013
 * This file is released into the public domain.
 */

import java.net.*;
import java.util.Arrays;
import ch.bildspur.artnet.*;

public class OPC
{
  ArtNetClient artnet;
  byte[][] dmxData;

  String host;
  int port;

  int[] pixelLocations;
  boolean enableShowLocations;

  int pixelsWidth;
  int pixelsHeight;

  int numUniverses;
  
  int valueToSend;

  OPC(PApplet parent, String host, int port)
  {
    this.host = host;
    this.port = port;
    this.enableShowLocations = true;
    parent.registerMethod("draw", this);



    artnet = new ArtNetClient(null);
    artnet.start();
  }

  // Set the location of a single LED
  void led(int index, int x, int y)
  {
    // For convenience, automatically grow the pixelLocations array. We do want this to be an array,
    // instead of a HashMap, to keep draw() as fast as it can be.
    if (pixelLocations == null) {
      pixelLocations = new int[index + 1];
    } else if (index >= pixelLocations.length) {
      pixelLocations = Arrays.copyOf(pixelLocations, index + 1);
    }
    pixelLocations[index] = x + width * y;
  }

  // Set the location of several LEDs arranged in a strip.
  // Angle is in radians, measured clockwise from +X.
  // (x,y) is the center of the strip.
  void ledStrip(int index, int count, float x, float y, float spacing, float angle, boolean reversed)
  {
    float s = sin(angle);
    float c = cos(angle);
    for (int i = 0; i < count; i++) {
      led(reversed ? (index + count - 1 - i) : (index + i),
        (int)(x + (i - (count-1)/2.0) * spacing * c + 0.5),
        (int)(y + (i - (count-1)/2.0) * spacing * s + 0.5));
    }
  }

  // Set the location of several LEDs arranged in a grid. The first strip is
  // at 'angle', measured in radians clockwise from +X.
  // (x,y) is the center of the grid.
  void ledGrid(int index, int stripLength, int numStrips, float x, float y,
    float ledSpacing, float stripSpacing, float angle, boolean zigzag)
  {
    float s = sin(angle + HALF_PI);
    float c = cos(angle + HALF_PI);
    for (int i = 0; i < numStrips; i++) {
      ledStrip(index + stripLength * i, stripLength,
        x + (i - (numStrips-1)/2.0) * stripSpacing * c,
        y + (i - (numStrips-1)/2.0) * stripSpacing * s, ledSpacing,
        angle, zigzag && (i % 2) == 1);
    }

    pixelsWidth=stripLength;
    pixelsHeight=numStrips;

    int individualLeds=pixelsHeight*pixelsWidth*3;

    numUniverses=floor(individualLeds/512);

    numUniverses+=individualLeds%512==0?0:1;

    dmxData = new byte[(numUniverses)][512];
    println("numUniverses:"+numUniverses);
  }

  // Automatically called at the end of each draw().
  // This handles the automatic Pixel to LED mapping.
  // If you aren't using that mapping, this function has no effect.
  // In that case, you can call setPixelCount(), setPixel(), and writePixels()
  // separately.
  void draw()
  {
    if (pixelLocations == null) {
      // No pixels defined yet
      return;
    }

    int numPixels = pixelLocations.length;
    int ledAddress = 0;
    int universeCounter = 0;

    loadPixels();

    for (int i = 0; i < numPixels; i++) {
      int pixelLocation = pixelLocations[i];
      int pixel = pixels[pixelLocation];

      dmxData[universeCounter][ledAddress] = (byte)(pixel >> 16);
      ledAddress++;
      if (ledAddress>511) {
        ledAddress=0;
        universeCounter++;
      }
      dmxData[universeCounter][ledAddress] = (byte)(pixel >> 8);
      ledAddress++;
      if (ledAddress>511) {
        ledAddress=0;
        universeCounter++;
      }
      dmxData[universeCounter][ledAddress] = (byte)(pixel);
      ledAddress++;
      if (ledAddress>511) {
        ledAddress=0;
        universeCounter++;
      }

      if (enableShowLocations) {
        pixels[pixelLocation] = 0xFFFFFF ^ pixel;
      }
    }
    dmxData[universeCounter][256] = (byte)(valueToSend);
   //println("sending value: "+valueToSend +" to univ "+universeCounter);
  //5184
  //5632

    writePixels();

    if (enableShowLocations) {
      updatePixels();
    }
  }

  // Transmit our current buffer of pixel values to the OPC server. This is handled
  // automatically in draw() if any pixels are mapped to the screen, but if you haven't
  // mapped any pixels to the screen you'll want to call this directly.
  void writePixels()
  {
    if (dmxData == null || dmxData.length == 0) {
      // No pixel buffer
      return;
    }


    try {
      int counter=0;
      for (int i = 0; i < pixelsHeight; i++) {
        artnet.unicastDmx(host, 0, i, dmxData[i]);
        counter++;
      }
        //artnet.unicastDmx(host, 0, 12, dmxData[11]);
         // artnet.broadcastDmx( 0, 13, dmxData[11]);
    }
    catch (Exception e) {
      dispose();
    }
  }

  void dispose()
  {
    // Destroy the socket. Called internally when we've disconnected.
  }
}
