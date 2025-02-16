#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <base64.h>  // You might need to include base64 library for encoding image data

#include "esp_camera.h"

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"

const char* ssid = "Device-Northwestern";

String apiKey = "";  // Use your Google Vision API key

WiFiClientSecure client;

HTTPClient http;

unsigned long lastCaptureTime = 0; // Last shooting time
int imageCount = 1;                // File Counter
bool camera_sign = false;          // Check camera status
bool sd_sign = false;              // Check sd status

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
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.frame_size = FRAMESIZE_QVGA;  // Options: FRAMESIZE_QQVGA, FRAMESIZE_QVGA, etc.
  config.jpeg_quality = 12;            // JPEG quality (lower is better quality)
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  camera_sign = true; // Camera initialization check passes
  
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  client.setInsecure();

  String url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash-lite-preview-02-05:generateContent?key=" + apiKey;

  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");
}

int testPictureCapture(){
  int errors = 0;
  for (int i=1; i<101; i++){
    camera_fb_t* new_pic = esp_camera_fb_get();
    if (!(new_pic->buf[0] == 0xff && new_pic->buf[1] == 0xd8 && new_pic->buf[2] == 0xff && new_pic->buf[3] == 0xe0)) {
      errors++;
      Serial.println("Error occured with image capture.");
    }
    // if(i % 10 == 0){
    //   Serial.print("Completed test: "); Serial.println(i);
    // }
    esp_camera_fb_return(new_pic);
  }
  // Serial.print("Total Errors: "); Serial.println(errors);
  return errors;
}

int testApiResponse(void) {
  int errors = 0;
  for (int i=0; i<10; i++) {
    // Serial.println(i);
    camera_fb_t* new_pic = esp_camera_fb_get();
    String encodedImage = base64::encode(new_pic->buf, new_pic->len);

    String body = "{\"contents\": [{\"parts\": [{\"text\": \"Give a one sentence description of this scene.\"}, {\"inline_data\": {\"mime_type\": \"image/jpeg\", \"data\": \"" + encodedImage + "\"}}]}]}";
    int httpResponseCode = http.POST(body);

    if (httpResponseCode != 200) {
      errors++;
    }

    esp_camera_fb_return(new_pic);
  }

  return errors;
}



void loop() {
  
  Serial.println("Beginning image capture test");

  int res = testPictureCapture();
  if (res > 0) {
    Serial.print("\tTest failed, percent failure: "); Serial.println(res);
  }
  else {
    Serial.println("\tTest passed, no errors");
  }

  Serial.println("\nBeginning API access test");

  unsigned long start = millis();
  res = testApiResponse();
  unsigned long end = millis();
  if (res > 0) {
    Serial.print("\tTest failed, percent failure: "); Serial.println(res);
  }
  else {
    Serial.println("\tTest passed, no errors");
  }
  Serial.print("\tAverage time for API response: "); Serial.print((float)(end-start)/10/1000); Serial.println("s");

  delay(30000);
}
