/*
  I2C VERSION
*/

#include <Wire.h>
#include <Servo.h> 
#include "MFRC522_I2C.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

// I2C communication
#define SDA_PIN D5
#define SCL_PIN D6
#define RST_PIN D3       

// Touch Sensor
#define SENSOR_PIN 4

// Servo angles
#define DOOR_LOCKED 10
#define DOOR_OPENED 170


MFRC522 mfrc522(0x28, RST_PIN);	   // Create MFRC522 instance.
Servo servo;                       // Creating Servo instance

// Server response and boolean
String response; 
bool isRegistered; 

// WiFi settings and Server URL
const char* ssid = "mipt-welcome"; 
const char* password = "";
String serverName = "http://10.55.139.35:3001/events/";

// Server URL path with parameters
String URLwParam;

// unsigned long lastTime = 0; // Add later
unsigned long timerDelay = 2000;


void setup() {
	Serial.begin(9600); 

  // WiFi connecting procedure
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP()); 
	
	Wire.begin(SDA_PIN, SCL_PIN);       // Initialize I2C	
  mfrc522.PCD_Init();		              // Init MFRC522	

  servo.attach(2, 500, 2400);         // attaching to D4 pin
  servo.write(DOOR_LOCKED); 

  // Init MFRC522
	mfrc522.PCD_Init();		              
  Serial.println("Ready for reading UIDs");

  pinMode(SENSOR_PIN, INPUT);  
}


void loop() {
  if(digitalRead(SENSOR_PIN) == HIGH) {
    Serial.println("Someone touched button!");
    openDoor(timerDelay);
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
    URLwParam = serverName + UID;
    
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
      
      openDoor(timerDelay);
      } else {
      for (int i = 0; i < keys.length(); i++) {
        JSONVar value = ParsedResponse[keys[i]];
        responseArr[i] = String(value); 
      }

      Serial.println("   Access Denied   ");
      Serial.print("Current time:");
      Serial.println(responseArr[1]); 
      }
    } 
}

void openDoor(unsigned long tmr) {
  servo.write(DOOR_OPENED);
  delay(tmr);
  servo.write(DOOR_LOCKED);
}

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