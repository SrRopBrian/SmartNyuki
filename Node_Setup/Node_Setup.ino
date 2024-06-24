#include "DHT.h"
#include "HX711.h"
#include <SPI.h>
#include <LoRa.h>
#include <Arduino.h>
#include <LowPower.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // Sensor type
#define ANALOGSOUNDPIN A0 // Analog pin connected to KY-037
#define LEDPIN 13 // Arduino LED pin
#define LOADCELL_DOUT_PIN 3 // DT pin of HX711
#define LOADCELL_SCK_PIN 4  // SCK pin of HX711

#define LORA_CS_PIN 10   // LoRa module CS pin
#define LORA_RST_PIN 9   // LoRa module RESET pin
#define LORA_IRQ_PIN 2   // LoRa module IRQ pin

#define POWER_PIN 7 

#define TRANSMITTER_ID 1 // Unique ID for the hive

DHT dht(DHTPIN, DHTTYPE);
HX711 scale;

int analogSoundVal;

void setup() {
  pinMode(ANALOGSOUNDPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  
  // Initialize sensors
  powerOnSensors();
  dht.begin(); 
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN); // HX-711
  
  // Initialize LoRa
  SPI.begin();
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_IRQ_PIN);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  readAndSendSensorData();
  sleepForMinutes(10);
}

void readAndSendSensorData() {
  powerOnSensors();
  delay(2000); // For sensors to stabilize

  // Read value from sensors
  analogSoundVal = analogRead(ANALOGSOUNDPIN);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float loadValue = scale.get_units(10); // Get average load value over 10 readings

  // Normalize the analog sound value to a range between 0 and 100
  float normalizedSound = analogSoundVal * (100.0 / 1023.0);
 
  // Check if any reads failed and exit early
  if (isnan(humidity) || isnan(temperature) || !scale.is_ready()) {
    Serial.println(F("Failed to read from sensors!"));
    powerOffSensors();
    return;
  }

  // Print sensor readings
  Serial.print(F("Sound value: "));
  Serial.print(analogSoundVal);
  Serial.print(F("\nSound Level: "));
  Serial.print(normalizedSound, 1); // Print with 1 decimal place for better resolution
  Serial.print("%");
  Serial.print(F("\nHumidity: "));
  Serial.print(humidity);
  Serial.print(F("% \nTemperature: "));
  Serial.print(temperature);
  Serial.print(F("°C \nLoad Cell: "));
  Serial.print(loadValue);
  Serial.print(F(" kg\n\n"));

  // Send data via LoRa
  LoRa.beginPacket();
  LoRa.print("ID: ");
  LoRa.print(TRANSMITTER_ID);
  LoRa.print(", Sound: ");
  LoRa.print(normalizedSound, 1);
  LoRa.print("%, Humidity: ");
  LoRa.print(humidity);
  LoRa.print("%, Temperature: ");
  LoRa.print(temperature);
  LoRa.print("°C, Load: ");
  LoRa.print(loadValue);
  LoRa.print(" kg");
  LoRa.endPacket();

  powerOffSensors();

  //Random Delay to minimize collisions
  delay(random(1000, 5000));
}

void powerOnSensors(){
  digitalWrite(POWER_PIN, HIGH);
}

void powerOffSensors(){
  digitalWrite(POWER_PIN, LOW);
}

void sleepForMinutes(int minutes) {
  for (int i = 0; i < minutes * 60/8; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}