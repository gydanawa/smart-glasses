/*
This code reads the Analog Voltage output from the
LV-MaxSonar sensors
If you wish for code with averaging, please see
playground.arduino.cc/Main/MaxSonar
Please note that we do not recommend using averaging with our sensors.
Mode and Median filters are recommended.
*/

#include "filter_lib.h"

#define THRESHOLD 36

// create filter with 2 Hz cut-off frequency
lowpass_filter lowpassFilter(2); 

float raw_signal;
float filtered_signal;

const int anPin1 = 3;
const int motorPin = 43;
const int cancelPin = 9;

double adc_value;
double distance1;
int threshold = 48;

double avg_distance;
double distances[5];
double filtered_distances[5];

//flags
bool obstacle_flag = 0;
bool cancel_button = 0; //if cancel flag is high, then do not vibrate motor
//cancel flag should negate any alarms for an object at close range until the button is pressed again to re-enable alerts

void setup() {
  Serial.begin(9600);  // sets the serial port to 9600
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
  Serial.print("Raw = ");
  for (int i=4; i>0; i--) {
    distances[i] = distances[i-1];
    Serial.print(distances[i]);
    Serial.print(", ");
  }
  distances[0] = distance1;

  
  //average the measurements
  Serial.print("Filtered = ");
  double total = 0.00;
  for (int i=0; i<5; i++) {
    //sum distances array
    filtered_distances[i] = lowpassFilter.filter(distances[i]);
    Serial.print(filtered_distances[i]);
    Serial.print(", ");
    total += filtered_distances[i];
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

void loop() {
  read_sensors();
  check_threshold();
  print_all();
  delay(50);
}
