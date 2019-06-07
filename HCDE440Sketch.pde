// HCDE 440 Final Project
// Display portion
// Alex Banh and Sunny Cui

import mqtt.*;

MQTTClient client;

// Values to store the data we recieve
String currTemp;
String AQI;
int wc;
float[] tempTrend;
String date;

// Integer to keep track of which screen we are currently displaying.
// 0 - Weather, 1 - Air Quality, 2 - Historical temperature
int currScreen;

// Images for our weather icons
PImage sunny;
PImage partlycloudy;
PImage overcast;
PImage cloudy;
PImage lightrain;
PImage mediumrain;
PImage warning;

void setup() {
  // Start the sketch in fullscreen and set the background color to black
  fullScreen();
  background(#000000);

  // Initialize our MQTT client and connect to the server with the username
  // hcdeiot and password esp8266, then subscribe to our topics
  client = new MQTTClient(this);
  client.connect("mqtt://hcdeiot:esp8266@mediatedspaces.net");
  client.subscribe("SunnyAlexFinal/wc");
  client.subscribe("SunnyAlexFinal/tempTrend");
  client.subscribe("SunnyAlexFinal/currTemp");
  client.subscribe("SunnyAlexFinal/AQI");
  client.subscribe("SunnyAlexFinal/date");

  // Initialize our data values with placeholder values so that we don't get
  // null errors later on
  this.date = "00/00/0000";
  this.currTemp = "0";
  this.wc = 113;
  this.AQI = "0";
  tempTrend = new float[11];
  for (int i = 0; i < 11; i++) {
    tempTrend[i] = 0.0;
  }

  // Load our weather icon images into memory
  sunny = loadImage("sunny.png");
  partlycloudy = loadImage("partlycloudy.png");;
  overcast = loadImage("overcast.png");;
  cloudy = loadImage("cloudy.png");;
  lightrain = loadImage("lightrain.png");;
  mediumrain = loadImage("mediumrain.png");;
  warning = loadImage("warning.png");;
}

// The main draw loop
void draw() {
  // Set the background to black each time, effectively cleaning the screen
  background(#000000);
  textSize(24);
  text(date, 650, 40);
  if (currScreen == 0) {  // Screen 0 - Weather display
    textSize(72);
    // Text for telling the current temperature
    text("It is currently \n" + currTemp + " degrees.", 50, 120);
    textSize(48);
    // If statement that displays a messasge depending on what the weather
    // condition is.
    if (wc == 119) {
      image(cloudy, 600, 100);
      text("It's looking to be a chill day.", 50, 400);
    } else if (wc == 113) {
      image(sunny, 600, 100);
      text("It's looking to be a great day!", 50, 400);
    } else if (wc == 116) {
      image(partlycloudy, 600, 100);
      text("It's looking to be a chill day.", 50, 400);
    } else if (wc == 122) {
      image(overcast, 600, 100);
      text("Great lighting for photos.", 50, 400);
    } else if (wc == 296) {
      image(lightrain, 600, 100);
      text("You might want a raincoat.", 50, 400);
    } else if (wc == 302) {
      image(mediumrain, 600, 100);
      text("You might want a raincoat.", 50, 400);
    } else {
      text("Error: Unknown Weather Code", 50, 400);
    }
  } else if (currScreen == 1) { // Screen 1 - Displays the air quality index
    textSize(72);
    text("The air quality index \nis ", 50, 120);
    int intAQI = Integer.parseInt(AQI);
    String message = "";
    // Displays a message depending on what the current AQI value is
    if (intAQI <= 50) {
      fill(#45F700);
      message = "The air's smelling fresh!";
    } else if (intAQI > 50 && intAQI <= 100) {
      fill(#F7F000);
      message = "The air's a little eh.";
    } else if (intAQI > 100 && intAQI <= 150) {
      fill(#FFA908);
      message = "Unhealthy for sensitive groups.";
    } else if (intAQI > 150 && intAQI <= 200) {
      fill(#FC3503);
      message = "This air can't be good for any of y'all.";
    } else if (intAQI > 200 && intAQI <= 300) {
      fill(#C103FC);
      message = "Yo, this is bad, real bad.";
    } else {
      fill(#CE1D35);
      message = "How the hell did we get here?";
    }
    text(AQI, 130, 227);
    fill(#FFFFFF);
    textSize(48);
    text(message, 50, 400);
  } else if (currScreen == 2) { // Screen 2 - Displays the historical temperature data
    textSize(38);
    text("Here's what the temperature was like \nduring previous years:" , 50, 90);
    strokeWeight(4);
    stroke(#FFFFFF);
    // Draws the axes for the graph
    line(50, 200, 50, 425);
    line(50, 425, 750, 425);
    textSize(32);
    int year = 2009;
    // Draws the year labels for the x-axis
    for (int i = 0; i < 6; i++) {
      text(year + (i*2), 50 + (122 * i), 465);
    }
    int temp = 20;
    // Draws the temperature labels for the y-axis
    for (int i = 0; i < 4; i++) {
      text(temp + (i * 20), 5, 425 - (60 * i));
      line(50, 425 - (60 * i), 750, 425 - (60 * i));
    }
    strokeWeight(6);
    stroke(#FF3700);
    // Draws the trend line based on the values recieved
    for(int i = 0; i < 10; i++) {
      line(90 + (i * 62), 467.5 - (tempTrend[i] * 2.375),
           90 + ((i + 1) * 62), 467.5 - (tempTrend[i + 1] * 2.375));
    }
  }
  delay(10000);
  currScreen++;
  if (currScreen == 3) {
    currScreen = 0;
  }
}

void mousePressed() {
  println(mouseX + ", " + mouseY + "\n");
}

// Listener for data from the MQTT server
void messageReceived(String topic, byte[] payload) {
  println("new message: " + topic + " - " + new String(payload));
  if (topic.equals("SunnyAlexFinal/wc")) {
    String temp = new String(payload);
    this.wc = Integer.parseInt(temp);
  } else if (topic.equals("SunnyAlexFinal/tempTrend")) {
    String input = new String(payload);
    this.tempTrend = float(split(input, ' '));
  } else if (topic.equals("SunnyAlexFinal/currTemp")) {
    this.currTemp = new String(payload);
  } else if (topic.equals("SunnyAlexFinal/AQI")) {
    this.AQI = new String(payload);
  } else if (topic.equals("SunnyAlexFinal/date")) {
    this.date = new String(payload);
  }
}
