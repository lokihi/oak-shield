#include <SPI.h>
#include <Servo.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>

#define RST_PIN         D1          
#define SS_PIN          D2         


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Creating MFRC522 instance
Servo servo;                       // Creating Servo instance

// Server response and parsed variant
String response; 
String responseArr[2]; 

const char* ssid = ""; 
const char* password = "";
const char* serverName = "http://172.20.10.3:3000/events/";

// Server URL path with parameters
String URLwParam;

void setup() {
	Serial.begin(9600); // optional

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
	
  SPI.begin();			
  servo.attach(2, 500, 2400);         // attaching to D4 pin
  servo.write(0); 

  // Init MFRC522
	mfrc522.PCD_Init();		              
  delay(500);
  Serial.println("Ready for reading UIDs");
}


void loop() {
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
  
  Serial.print("Tag UID :");

  for(byte i = 0; i < mfrc522.uid.size; i++) {
    // Displaying UID into Serial Monitor
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    // Writing UID as string
    UID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    UID.concat(String(mfrc522.uid.uidByte[i], HEX));

    delay(50);      // optional delay
  }
  UID.toUpperCase(); 

  // Halt PICC (stops reading)
  mfrc522.PICC_HaltA();

  if (WiFi.status()==WL_CONNECTED){
    URLwParam = server + UID.substring(1);
    response = httpGETRequest(URLwParam);
    JSONVar ParsedResponse = JSON.parse(response);

    // There could be a check on whether parsing was successful

    // Getting an array of all the keys 
    JSONVar keys = ParsedResponse.keys();
    // Writing response to an array
    for (int i = 0; i < keys.length(); i++) {
      JSONVar value = ParsedResponse[keys[i]];
      responseArr[i] = String(value); 
    }
  }

  if (responseArr[0])       // check if student has registered  
  {
    Serial.println("   Access Granted   ");

    // Additional information from server:
    // Serial.print("Current time:");
    // Serial.println(responseArr[1]); 
    
    // Servo rotates 90 degrees for 1 sec
    servo.write(90);
    delay(1000);
    servo.write(0);
    // Optional delay
    delay(1000);
  }
  
  else   {
    Serial.println("   Access Denied   ");
    // Optional delay
    delay(1000);
  }
}


String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, serverName);
  
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
