// HCDE 440 Final Project
// Ambient weather display
// Alex Banh and Sunny Cui

//Requisite Libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Adafruit_MPL115A2.h>
#include <DHT.h>

#define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321

#define DHTPIN 12 // Digital pin connected to the DHT sensor, change this if you're using a different pin!

#define wifi_ssid "YOUR_WIFI_HERE" // name of the Wifi I'm connecting to
#define wifi_password "YOUR_WIFI_HERE"  // password of the Wifi I'm connecting to

// MQTT
#define mqtt_server "mediatedspaces.net"       //this is its address, unique to the server
#define mqtt_user "hcdeiot"                    //this is its server login, unique to the server
#define mqtt_password "esp8266"   //this is it server password, unique to the server

// Strings to store the current date
String currYear;
String currMon;
String currDay;

// Initialize DHT sensor 
DHT dht(DHTPIN, DHTTYPE);

Adafruit_MPL115A2 mpl115a2;

WiFiClient espClient;                          //blah blah blah, espClient
PubSubClient mqtt(espClient);                  //blah blah blah, tie PubSub (mqtt) client to WiFi client

// BreezoMeter API
// REPLACE THIS API KEY WITH YOUR OWN KEY
const char* key = "YOUR_KEY_HERE"; // API key from BreezoMeter

// Variable decelerations
// Seattle's latitude and longitude: 47.6062° N, 122.3321° W
const char* lat = "47.6062";    // latitude
const char* lon = "-122.3321";  // longitude
unsigned long timer;            // holding values to the timer 

const char* host = "api.breezometer.com";   // API url
const int httpsPort = 443;                  // API https port

// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "0C A9 23 4F 27 B3 17 DA AD E7 4C 0B EA 61 C5 84 F2 8D C3 DA";

char mac[6];       // unique id
char message[201]; // setting message length

// Setup Wifi
void setup_wifi() {
  delay(10);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");         //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());         //.macAddress returns a byte array 6 bytes representing the MAC address
  WiFi.macAddress().toCharArray(mac, 6);     //5C:CF:7F:F0:B0:C1 for example
}

// function to reconnect if we become disconnected from the server
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection..."); // print out "Attempting MQTT connection..."
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");        // print out "connected"
      mqtt.subscribe("theSunnyTopic/+");  // subscribe to "theSunnyTopic/+"
    } else {
      Serial.print("failed, rc=");               // print out "failed, rc="
      Serial.print(mqtt.state());                // print out the state of MQTT
      Serial.println("try again in 5 seconds");  // print out "try again in 5 seconds"
      delay(5000);                               // wait 5 seconds before retrying
    }
  }
}

void setup() {
  Serial.begin(115200); 


  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));

  while (! Serial);

  setup_wifi();                      //start wifi

  mqtt.setServer(mqtt_server, 1883); //start the mqtt server
  mqtt.setCallback(callback);        //register the callback function

  timer = millis();

  mpl115a2.begin(); // initialize barometric sensor
  dht.begin();

  Serial.println(currMon + "/" + currDay + "/" + currYear);
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop(); //this keeps the mqtt connection 'active'

  getDate();

  getMet();  

  getTodayWeather();

  int startYear = 2009; // The year to start gathering historical data from (any year after and including 2009)

  String tempTrend = "";

  // Gets the historical temperature for each year from 2009 to 2019 and
  // concatenates it into a string with each value separated by a space.
  for (int i = 0; i < 11; i++) {
    tempTrend += getHistWeather(String(startYear + i)) + " ";
  }

  Serial.println(tempTrend);

  tempTrend.toCharArray(message, 201);
  mqtt.publish("SunnyAlexFinal/tempTrend", message);

  getAmbWeather();
  
  delay(60000);  // wait for a minute before sending more information
}


void getMet() {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url = (String("https://api.breezometer.com/air-quality/v2/current-conditions?lat=")
                + lat + "&lon=" + lon + "&key=" + key);            // API url
  Serial.print("requesting URL: ");                                // print out "requesting URL: "
  Serial.println(url);                                             // print out the API url

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');                    // read the json file
    if (line == "\r") {                                            // if there is a carriage return
      Serial.println("headers received");                          // print out "headers received"
      break;
    }
  }
  String line = client.readStringUntil('\n');                      // initialize a string called "line" that read the json file
  if (line.startsWith("{\"state\":\"success\"")) {                 // if "line" start with {"state":"success"
    Serial.println("esp8266/Arduino CI successful!");             // print out "esp8266/Arduino CI successfull!"
  } else {
    Serial.println("esp8266/Arduino CI has failed");               // print out "esp8266/Arduino CI has failed"
  }
  Serial.println("reply was:");                                    // print out "reply was:"
  Serial.println("==========");                                    // print out "=========="
  Serial.println(line);                                            // print out "line" (json data from BreezoMeter)
  Serial.println("==========");                                    // print out "=========="
  Serial.println("closing connection");                            // print out "closing connection"

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parse(line);

  String aqi = root["data"]["indexes"]["baqi"]["aqi"].as<String>();// initialize a string called "aqi" that stores the Air Quality Index from the MQTT
  Serial.println("Air quality index: " + aqi);                                             // print out aqi

  sprintf(message, "%s", aqi.c_str());// prints the massage in json format
  mqtt.publish("SunnyAlexFinal/AQI", message);                     // publish the message to "theSunnyTopic/AQI"
}

// Uses the world weather online API to get the weather code (rain, sun, cloudy, etc) for today.
void getTodayWeather() {
  HTTPClient theClient;
  theClient.begin("http://api.worldweatheronline.com/premium/v1/past-weather.ashx?key=YOUR_KEY_HERE&q=Seattle&format=json&date=today&includelocation=no&show_comments=no&tp=24");
  int httpCode = theClient.GET();
  Serial.println(httpCode);
  if (httpCode > 0) {
    if (httpCode == 200) {
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }
      String histTemp = root["data"]["weather"][0]["maxtempF"].asString();
      String weatherCode = root["data"]["weather"][0]["hourly"][0]["weatherCode"],asString();
      Serial.println("Historical temperature: " + histTemp + " WC: " + weatherCode);
      weatherCode.toCharArray(message, 201);
      mqtt.publish("SunnyAlexFinal/wc", message);
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

// Given a year to grab data from, get the average temperature from today's day on that year.
String getHistWeather(String histYear) {
  HTTPClient theClient;
  theClient.begin("http://api.worldweatheronline.com/premium/v1/past-weather.ashx?key=YOUR_KEY_HERE5&q=Seattle&format=json&date=" + histYear + "-" + currMon + "-" + currDay + "&includelocation=no&show_comments=no&tp=24");
  int httpCode = theClient.GET();
  if (httpCode > 0) {
    if (httpCode == 200) {
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return "Error";
      }
      String histTemp = root["data"]["weather"][0]["maxtempF"].asString();
      Serial.println("Historical temperature for " + histYear + ": " + histTemp);
      return histTemp;
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
      return "";
    }
  }
}

// Grabs the ambient temperature from the DHT sensor
void getAmbWeather() {

  // ======== START DHT =========
  
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }

  // Print out all the information collected from the DHT sensor

  Serial.print(f);
  Serial.print(F("°F");

  String currTemp = String(f);
  currTemp = currTemp.substring(0, 2);
  currTemp.toCharArray(message, 201);
  mqtt.publish("SunnyAlexFinal/currTemp", message);
}

// Gets the current date
void getDate() {
  HTTPClient theClient;
  theClient.begin("http://worldclockapi.com/api/json/est/now");
  int httpCode = theClient.GET();
  Serial.println(httpCode);
  if (httpCode > 0) {
    if (httpCode == 200) {
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);
      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }
      String currDate = root["currentDateTime"].asString();
      
      currYear = currDate.substring(0, 4);
      currMon = currDate.substring(5, 7);
      currDay = currDate.substring(8, 10);
      Serial.println("Current Date: " + currMon +"/" + currDay + "/" + currYear);
      String prettyDate = currMon +"/" + currDay + "/" + currYear;
      prettyDate.toCharArray(message, 201);
      mqtt.publish("SunnyAlexFinal/date", message);
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
}

// The callback is where we attach a listener to the incoming messages from the server
void callback(char* topic, byte* line, unsigned int timer) {
  Serial.println();                  // print out a blank line
  Serial.print("Message arrived ["); // print out "Message arrived [" 
  Serial.print(topic);               // print out topic
  Serial.println("] ");              // print out "] "

  DynamicJsonBuffer  jsonBuffer;                   // json buffer
  JsonObject& root = jsonBuffer.parseObject(line); // parse it

  if (!root.success()) { // if parsing is not success
    Serial.println("parseObject() failed, are you sure this message is JSON formatted."); // print out the message
    return;
  }
}
