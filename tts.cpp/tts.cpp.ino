#include <WiFi.h>
#include "HTTPClient.h"
#include "Audio.h"

//wifi credentials
const char* ssid = "Device-Northwestern";

//I2S pin definitions
#define I2S_DOUT 25
#define I2S_BCLK 33
#define I2S_LRC 32

Audio audio;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to Wifi");

  //initialize I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  String text = "Hello, I am the ESP32.";
  playLongText(text);

}

void loop() {
  // put your main code here, to run repeatedly:
  audio.loop();

}

void playLongText(String text) {
  int maxChunkSize = 200;
  int startPos = 0;

  while (startPos < text.length()) {
    int endPos = min(startPos + maxChunkSize, (int)text.length());
    String chunk = text.substring(startPos, endPos);

    //URL encode the text chunk
    chunk.replace(" ", "%20");

    //construct the URL for Google TTS
    String tts_url = "http://translate.google.com/translate_tts?ie=UTF-8&q=" + chunk + "&tl=en&client=tw-ob";
    
    //play the audio
    audio.connecttohost(tts_url.c_str());

    //wait for the chunk to finish playing
    while (audio.isRunning()) {
      audio.loop();
    }

    startPos = endPos;
    delay(500);
  }
}
