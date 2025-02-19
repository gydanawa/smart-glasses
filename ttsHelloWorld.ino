/* Uses ESP32-audioI2S Library - https://github.com/schreibfaul1/ESP32-audioI2S

need to download version 3.07 of ESP32-audioI2S*/
#include <Arduino.h>
#include <google-tts.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Audio.h"

// Define I2S connections
#define I2S_DOUT  6
#define I2S_BCLK  4
#define I2S_LRC   5

// Wifi Credentials
//String ssid =    "Device-Northwestern";
//Nicole Home
/*String ssid =    "BS_WIFI_EXT";
String password = "Horalka123";*/
String ssid =    "Device-Northwestern";

// Create an Audio object
Audio audio;

void setup() {
  Serial.begin(115200);

  // Setup WiFi in Station mode
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid.c_str(), password.c_str());
  WiFi.begin(ssid.c_str());
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // WiFi Connected, print IP to serial monitor
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  // Initialize I2S audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(17);          // Volume from 0 to 21
  
  TTS tts;
  String urlString = tts.getSpeechUrl("Hello, World!");
  const char* mp3URL = urlString.c_str();
  Serial.println(mp3URL);
  // Connect to the stream
  audio.connecttohost(mp3URL);
}

void loop() {
  // Handle audio streaming
  audio.loop();
}
