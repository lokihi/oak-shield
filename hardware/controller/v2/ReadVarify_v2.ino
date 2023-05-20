#include <Wire.h>
#include <Servo.h> 
#include "MFRC522_I2C.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Preferences.h>


// LED strip
#define NUM_LEDS 12
#include "FastLED.h"
#define PIN 5 // attaching to D1
CRGB leds[NUM_LEDS];


// I2C communication
#define SDA_PIN D5
#define SCL_PIN D6
#define RST_PIN D3       

// Touch Sensor
#define SENSOR_PIN 4

// Servo velocities
#define DOOR_LOCKED 75
#define DOOR_OPENED 105

Preferences preferences;

// ESP's ssid & password
const char* ssid_esp = "NodeMCU"; 
const char* password_esp = "12345678";

// NodeMCU WebServer configuration
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
ESP8266WebServer server(80);

// WiFi settings and Server IP from initial setup
String ssid;
String password;
String serverIP;


MFRC522 mfrc522(0x28, RST_PIN);	   // Create MFRC522 instance.
Servo servo;                       // Creating Servo instance

// Server response and boolean
String response; 
bool isRegistered; 

// Server URL path with parameters
String URLwParam;


void setup() {
  Serial.begin(9600); 

  // Init sensor
  pinMode(SENSOR_PIN, INPUT); 

  // Init LED strip
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(50);
  pinMode(13, OUTPUT);

  preferences.begin("wifi-configuration", false);

  ssid = preferences.getString("name", "");
  password = preferences.getString("password", "");
  serverIP = preferences.getString("ip", "");


  if (ssid == "") {
    setupLED();
    initial_setup();

    Serial.println(ssid);
    Serial.println(password);
    Serial.println(serverIP);
  } else {
    if (digitalRead(SENSOR_PIN) == HIGH){
      setupLED();
      initial_setup();

      Serial.println(ssid);
      Serial.println(password);
      Serial.println(serverIP);
    }
  }

  // WiFi connecting procedure
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  wifiSetupLED();
  delay(7000); 
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); 
	
	Wire.begin(SDA_PIN, SCL_PIN);       // Initialize I2C	
  mfrc522.PCD_Init();		              // Init MFRC522	

  servo.attach(13);         // attaching to D7 pin

  // Init MFRC522
	mfrc522.PCD_Init();		              
  Serial.println("Ready for reading UIDs");

  preferences.end();
}


void loop() {
  if(digitalRead(SENSOR_PIN) == HIGH) {
    Serial.println("Someone touched button!");
    openDoorLED();
    return;
  }

	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

  // Readable tag
  String UID = "";
  
  Serial.print("Card UID :");

  for(byte i = 0; i < mfrc522.uid.size; i++) {
    // Displaying UID into Serial Monitor
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    // Writing UID as string
    UID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    UID.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  UID.toUpperCase(); 

  // Halt PICC (stops reading)
  mfrc522.PICC_HaltA();

  if (WiFi.status()==WL_CONNECTED){
    // URLwParam = serverName + UID;
    URLwParam = "http://" + serverIP + "/events/" + UID;
    
    response = httpGETRequest(URLwParam.c_str());
    JSONVar ParsedResponse = JSON.parse(response);

    // There could be a check on whether parsing was successful
    if (JSON.typeof(ParsedResponse) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }

    Serial.print("Response Info = ");
    Serial.println(ParsedResponse);

    // Getting an array of all the keys 
    JSONVar keys = ParsedResponse.keys();
    isRegistered = ParsedResponse[keys[0]];
    String responseArr[keys.length()];

  
    if (isRegistered) { // if user has registered
      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = ParsedResponse[keys[i]];
        responseArr[i] = String(value); 
      }

      Serial.println("   Access Granted   ");
      // Additional information from server:
      Serial.println("Welcome, " + responseArr[1] + " " + responseArr[2]);
      Serial.print("Current time:");
      Serial.println(responseArr[3]); 
      
      openDoorLED();
      } else {
      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = ParsedResponse[keys[i]];
        responseArr[i] = String(value); 
      }

      Serial.println("   Access Denied   ");
      Serial.print("Current time:");
      Serial.println(responseArr[1]); 
      DoorClosed();
      }
    } 
}

// LED indication functions
void PositiveLED(){
  for(int i = 0; i< NUM_LEDS; i++){
    for(int j = 1; j<255;){
      leds[i] = CHSV(85, 255, j);
      FastLED.show();
      j += 10;
    }
  }

  for(int i = NUM_LEDS - 1; i >= 0;){
    delay(700);

    for(int j = 255; j>0;){
      leds[i] = CHSV(85, 255, j);
      leds[i-1] = CHSV(85, 255, j);
      FastLED.show();
      j -= 4;
    }
    
    leds[i] = CHSV(85, 255, 0);
    leds[i-1] = CHSV(85, 255, 0);

    i -= 2;
  }
  FastLED.show();
}

void DoorClosed() {
  for(int counter = 0; counter < 3; counter++){
    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CHSV(0, 255, 255);
      FastLED.show();
    }

    delay(200);

    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CHSV(0, 0, 0);
      FastLED.show();
    }

    delay(300);
  }
}

void openDoorLED() {
  servo.write(DOOR_OPENED); 
  delay(300);
  servo.write(90);

  PositiveLED();

  servo.write(DOOR_CLOSED);
  delay(300);
}

void wifiSetupLED(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CHSV(165, 255, 255); // blue color
    FastLED.show();
  }
  
  delay(200);

  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CHSV(0, 0, 0);
    FastLED.show();
  }

  delay(200);
}

void setupLED() {
  for(int counter = 0; counter < 3; counter++){
    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CHSV(165, 255, 255); // blue color
      FastLED.show();
    }

    delay(200);

    for(int i = 0; i < NUM_LEDS; i++){
      leds[i] = CHSV(0, 0, 0);
      FastLED.show();
    }

    delay(200);
  }
}

// GET request function
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);
  http.addHeader("authorization", "fantastic_pass");
  
  // Send HTTP GET request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

// Initial setup & ESP WebServer functions
void initial_setup() {
  WiFi.softAP(ssid_esp, password_esp);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  server.on("/", HTTP_GET, handleRoot);        // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/login", HTTP_POST, handleLogin); // Call the 'handleLogin' function when a POST request is made to URI "/login"
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  while(true) {server.handleClient();}
}

 
void handleRoot() {
  server.send(200, "text/html", SendHTML2());
}

void handleLogin() {
  ssid = server.arg("ssid");
  password = server.arg("password");
  serverIP = server.arg("serverip");

  preferences.putString("name", ssid);
  preferences.putString("password", password);
  preferences.putString("ip", serverIP);

  server.send(200, "text/html", "<form <br>Authorization completed, restarting...></form>");
  delay(1000);
  ESP.restart();
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


String SendHTML2() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<body>\n";

  ptr += "<div style=\"margin: 0; position: absolute; top: 50%; bottom: 50%; transform: translate(-50%, -50%);\">";
  ptr += "<form action=\"/login\" method=\"POST\">";
  ptr += "<center><input style=\"height: 60px; width: 280px; font-size: 1.7em; text-align: center;\" type=\"text\" name=\"ssid\" required=\"required\" placeholder=\"WiFi Network\"></br></center>";
  ptr += "<center><input style=\"height: 60px; width: 280px; font-size: 1.7em; text-align: center;\" type=\"text\" name=\"password\" required=\"required\" placeholder=\"Password\"></br></center>"; 
  ptr += "<center><input style=\"height: 60px; width: 280px; font-size: 1.7em; text-align: center;\" type=\"text\" name=\"serverip\" required=\"required\" placeholder=\"Server IP\"></br></center>";
  ptr += "<center><input style=\"position:relative; top:10px; color:white; border-radius: 15px; background-color: #395BBF; height: 70px; width: 100px; font-size: 1em; text-align: center;\" type=\"submit\" value=\"Authorize\"></center>";
  
  ptr += "</form>\n";
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

