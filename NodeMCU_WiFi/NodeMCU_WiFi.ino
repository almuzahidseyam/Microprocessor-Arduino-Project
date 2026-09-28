/*
 * Smart Biometric & IoT Door Lock System - WiFi Module
 * ----------------------------------------------------
 * Controller: NodeMCU ESP8266
 * Function: Connects to local WiFi and hosts a Web Server
 *           to remotely unlock the door. Sends signal to 
 *           the main Arduino Uno/Nano.
 * 
 * Auto-generated Boilerplate.
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ==========================================
// WIFI CREDENTIALS
// ==========================================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ==========================================
// SYSTEM SETUP
// ==========================================
ESP8266WebServer server(80);

// Pin to signal the main Arduino to unlock the door
#define SIGNAL_PIN D1 

void setup() {
  Serial.begin(115200);
  delay(10);
  
  pinMode(SIGNAL_PIN, OUTPUT);
  digitalWrite(SIGNAL_PIN, LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup Web Server Routes
  server.on("/", handleRoot);
  server.on("/unlock", handleUnlock);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

// ==========================================
// WEB SERVER HANDLERS
// ==========================================

void handleRoot() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family: Arial; text-align: center; margin-top: 50px;}";
  html += "button{background-color: #4CAF50; border: none; color: white; padding: 15px 32px; ";
  html += "text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border-radius: 10px;}</style></head>";
  html += "<body><h1>Smart Door Lock</h1>";
  html += "<p>Status: <strong>Secured</strong></p>";
  html += "<a href=\"/unlock\"><button>Unlock Door</button></a>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleUnlock() {
  Serial.println("Remote Unlock Triggered!");
  
  // Send a HIGH pulse to the Arduino to unlock the door
  digitalWrite(SIGNAL_PIN, HIGH);
  delay(1000); // 1 second pulse
  digitalWrite(SIGNAL_PIN, LOW);
  
  // Return to home page
  server.sendHeader("Location", "/");
  server.send(303);
}
