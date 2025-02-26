#include <Audio.h>

#include <Arduino.h>
#include <google-tts.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <base64.h>  // You might need to include base64 library for encoding image data

#include "esp_camera.h"

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"
#include "Audio.h"

// Define I2S connections
#define I2S_DOUT  6
#define I2S_BCLK  4
#define I2S_LRC   5

const char* ssid = "Device-Northwestern";

String apiKey = "";  // Use your Google Vision API key

WiFiClientSecure client;

HTTPClient http;

bool send_image_flag = false;
bool wait_for_audio = false;
unsigned long startTime = 0;
unsigned long stopLength = 0;

#define IMAGE_PIN 9

Audio audio;
TTS tts;

void ARDUINO_ISR_ATTR image_callback(void) {
  send_image_flag = true;
}

void init_interrupt(void) {
  pinMode(IMAGE_PIN, INPUT_PULLUP);
  attachInterrupt(IMAGE_PIN, image_callback, FALLING);
}

void setup() {
  Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.frame_size = FRAMESIZE_QVGA;  // Options: FRAMESIZE_QQVGA, FRAMESIZE_QVGA, etc.
  config.jpeg_quality = 12;            // JPEG quality (lower is better quality)
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }
  
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setInsecure();

  // HTTPClient http;
  String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash-lite:generateContent?key=" + apiKey;

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
    
  init_interrupt();
  Serial.println("Button interrupt initialized");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(17);
}

void send_image(void) {
  Serial.println("Function!");
  camera_fb_t* new_pic = esp_camera_fb_get();

  String encodedImage = base64::encode(new_pic->buf, new_pic->len);

  String body = "{\"contents\": [{\"parts\": [{\"text\": \"Give a one sentence description of this scene.\"}, {\"inline_data\": {\"mime_type\": \"image/jpeg\", \"data\": \"" + encodedImage + "\"}}]}]}";
  
  int httpResponseCode = http.POST(body);

  Serial.println(httpResponseCode);

  String description;

  if (httpResponseCode == 200) {
    String response = http.getString();
    int descriptionStart = response.indexOf("\"text\": \"") + 9;
    int descriptionEnd = response.indexOf("\\n", descriptionStart);
    description = response.substring(descriptionStart, descriptionEnd);
    Serial.println("Image description: " + description);
  } else {
    Serial.println("ERROR WITH IMAGE: " + httpResponseCode);
  }

  esp_camera_fb_return(new_pic);
  
  String urlString = tts.getSpeechUrl(description);
  const char* mp3URL = urlString.c_str();
  audio.connecttohost(mp3URL);
  wait_for_audio = true;
  startTime = millis();
  stopLength = description.length()*1000/12;
}

void loop() {
  if (send_image_flag) {
    send_image();
    send_image_flag = false;
  }
  audio.loop();
  unsigned long now = millis();
  if (wait_for_audio && (now - startTime > stopLength)) {
    Serial.println(audio.getFileSize());
    Serial.println(audio.getFilePos());
    Serial.println(audio.getTotalPlayingTime());
    wait_for_audio = false;
    audio.stopSong();
  }
}
