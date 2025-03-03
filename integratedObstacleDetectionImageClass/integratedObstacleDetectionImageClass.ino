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
#define THRESHOLD 36

const int anPin1 = 3;
const int motorPin = 43;
const int cancelPin = 9;

double adc_value;
double distance1;
int threshold = 48;

double avg_distance;
double distances[5];

String obstacleDetectedMessage = "Obstacle detected!";

//flags
bool obstacle_flag = 0;
bool cancel_button = 0;

const char* ssid = "Device-Northwestern";

String apiKey = "";  // Use your Google Vision API key

WiFiClientSecure client;

HTTPClient http;

bool send_image_flag = false;
bool wait_for_audio = false;
unsigned long startTime = 0;
unsigned long stopLength = 0;

#define IMAGE_PIN 7

Audio audio;
TTS tts;

void ARDUINO_ISR_ATTR image_callback(void) {
  send_image_flag = true;
}

void init_interrupt(void) {
  pinMode(IMAGE_PIN, INPUT_PULLDOWN);
  attachInterrupt(IMAGE_PIN, image_callback, RISING);
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
  analogReadResolution(12);
  pinMode(anPin1, INPUT);
  pinMode(cancelPin, INPUT_PULLUP);
  pinMode(motorPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(cancelPin), ISR_button_pressed, FALLING);
}
void ISR_button_pressed(void) 
{
  //toggle the cancel button signal
  cancel_button = !cancel_button;
  Serial.println("interrupt triggered");
  Serial.println(cancel_button);
}
void check_threshold() {
  //check threshold
  if (avg_distance < THRESHOLD) {
    Serial.println("OBSTACLE DETECTED!");
    obstacle_flag = 1;
    
  } else {
    obstacle_flag = 0;
  }

  //control the motor output
  if (obstacle_flag && !cancel_button) {
    analogWrite(motorPin, 125);
    Serial.println("motor on");

    //send the audio message
    String urlString = tts.getSpeechUrl(obstacleDetectedMessage);
    const char* mp3URL = urlString.c_str();
    audio.connecttohost(mp3URL);
    wait_for_audio = true;
    startTime = millis();
    stopLength = description.length()*1000/12;
  }
  else if (!obstacle_flag || cancel_button) {
    analogWrite(motorPin, 0);
  }
}
void read_sensors() {
  /*
  Scale factor is (Vcc/512) per inch. A 5V supply yields ~9.8mV/in
  */
  //XIAO ESP32S3 goes from 0 to 4095, so divide by 8 to get inches
  //distance1 = analogRead(anPin1)/8;
  adc_value = analogRead(anPin1);
  distance1 = (adc_value * 512.0) / 4096.0 /2; //20 in = 45; 25 in = 50; 30 in = 60, 40 in = 68 almost 70

  //update distances vector
  for (int i=4; i>0; i--) {
    distances[i] = distances[i-1];
  }
  distances[0] = distance1;

  //average the measurements
  double total = 0.00;
  for (int i=0; i<5; i++) {
    //sum distances array
    total += distances[i];
  }
  avg_distance = total/5;
  
  
}
void print_all() {
  // Serial.print("S1");
  // Serial.print(" ");
  // Serial.println(distance1);
  // Serial.print(" inches; ADC val: ");
  // Serial.print(adc_value);
  // Serial.println();
  Serial.print("Average distance:");
  Serial.print(avg_distance);
  Serial.print(",");
  // Serial.println(obstacle_flag);
  Serial.print("Threshold:");
  Serial.println(threshold);
  Serial.print("Cancel button state: ");
  Serial.println(cancel_button);
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
    //Serial.println(audio.getFileSize());
    //Serial.println(audio.getFilePos());
    //Serial.println(audio.getTotalPlayingTime());
    wait_for_audio = false;
    audio.stopSong();
  }
  read_sensors();
  check_threshold();
  print_all();
  //delay(50);
}