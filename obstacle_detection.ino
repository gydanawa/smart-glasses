/*
This code reads the Analog Voltage output from the
LV-MaxSonar sensors
If you wish for code with averaging, please see
playground.arduino.cc/Main/MaxSonar
Please note that we do not recommend using averaging with our sensors.
Mode and Median filters are recommended.
*/

#define THRESHOLD 60

const int anPin1 = 3;
double adc_value;
double distance1;
int threshold = 60;

double avg_distance;
double distances[5];

bool obstacle_flag = 0;

void setup() {
  Serial.begin(9600);  // sets the serial port to 9600
  analogReadResolution(12);
  pinMode(anPin1, INPUT);
}

void read_sensors() {
  /*
  Scale factor is (Vcc/512) per inch. A 5V supply yields ~9.8mV/in
  */
  //XIAO ESP32S3 goes from 0 to 4095, so divide by 8 to get inches
  //distance1 = analogRead(anPin1)/8;
  adc_value = analogRead(anPin1);
  distance1 = (adc_value * 512.0) / 4095.0;

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
  
  //check threshold
  if (avg_distance < THRESHOLD) {
    Serial.println("OBSTACLE DETECTED!");
    obstacle_flag = 1;
  } else {
    obstacle_flag = 0;
  }
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
}

void loop() {
  read_sensors();
  print_all();
  delay(100);
}
