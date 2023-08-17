#include <HX711.h>
#include <max6675.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>

#define LOAD_CELL_DOUT_PIN A2
#define LOAD_CELL_CLK_PIN A3
#define THERMOCOUPLE_data 13
#define THERMOCOUPLE_clk 12
#define THERMOCOUPLE_cs 11
#define VIBRATION_SENSOR_PIN A1
#define IGNITER_PIN 8
#define LED_PIN 9



HX711 scale;
MAX6675 thermocouple(THERMOCOUPLE_clk,THERMOCOUPLE_cs,THERMOCOUPLE_data);

int count = 0;
bool abortSignal = false;

void setup() {
  Serial.begin(57600);
  pinMode(IGNITER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(7,OUTPUT); 


  scale.begin(LOAD_CELL_DOUT_PIN, LOAD_CELL_CLK_PIN);
//  thermocouple.begin(THERMOCOUPLE_PIN);
 // SD.begin(7)
  if (!SD.begin(7)) {
    Serial.println("SD card initialization failed!");
    //while (1);
  }
  // If abort signal is LOW, increase the count
  if (abortSignal == LOW) {
    count++;
  }
}



void loop() {
  // Check for abort signal every 2 seconds
  if (millis() % 2000 == 0) {
    if (abortSignal == HIGH) {
      Serial.println("Abort signal detected!");
    }
  }

  
  // Check if count limit is reached
  if (count == 5) {
    igniteMotor();
    Serial.println("Motor Ignition Started"); 
  }

  // Collect and process data every 20 milliseconds
  if (millis() % 20 == 0) {
    collectAndProcessData();
  }
}

void igniteMotor() {
  // Code to ignite the motor
  digitalWrite(IGNITER_PIN, HIGH);

  // Reset count
  count = 0;
}

void collectAndProcessData() {
  // Thermocouple
  float temperature = thermocouple.readCelsius();
  if (temperature > 1450) {
    flashLED();
    abortSignal = true;
  }
  else {
    storeData(temperature, scale.read(), analogRead(VIBRATION_SENSOR_PIN));
    Serial.println("logging temp");
    Serial.println(thermocouple.readCelsius());
  }

  // Load cell
  int loadCellData = scale.read();
  if (loadCellData > 500) {
    flashLED();
    abortSignal = true;
  }
  else {
    storeData(temperature, loadCellData, analogRead(VIBRATION_SENSOR_PIN));
    Serial.println("logging loadcell data");
    Serial.println(scale.read());
  }

  // Vibration sensor
  int vibrationData = analogRead(VIBRATION_SENSOR_PIN);
  if (vibrationData > 500) {
    flashLED();
    abortSignal = true;
  }
  else {
    storeData(temperature, loadCellData, vibrationData);
    Serial.println("logging vibration data");
    Serial.println(vibrationData);
  }
}

void storeData(float temperature, int loadCellData, int vibrationData) {
  // Code to store data in SD card
  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(temperature);
    dataFile.print(",");
    dataFile.print(loadCellData);
    dataFile.print(",");
    dataFile.println(vibrationData);
    dataFile.close();
  }
  else {
    Serial.println("Error opening data file!");
  }
}

void flashLED() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
