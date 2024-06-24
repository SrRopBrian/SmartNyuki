#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// LoRa Pin Definitions
#define LORA_SS 18
#define LORA_RST 14
#define LORA_DIO0 26

// WiFi credentials
#define WIFI_SSID "WiFi_SSID"
#define WIFI_PASSWORD "WiFi_PASSWORD"

// Firebase credentials
#define API_KEY "your_FIREBASE_API_KEY"
#define DATABASE_URL "https://your-database-name.firebaseio.com/"

// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("Connected to WiFi");
}

// Initialize Firebase
void initFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void setup() {
  Serial.begin(9600);

  // Initialize LoRa module
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(915E6)) {  // Use the correct frequency for your region (e.g., 868E6 in Europe)
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Receiver");

  // Initialize WiFi and Firebase
  initWiFi();
  initFirebase();
}

void loop() {
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    Serial.print("Received packet '");

    // Read packet
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    Serial.print(incoming);

    // Process the incoming data
    processData(incoming);

    // Print RSSI (Received Signal Strength Indicator)
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}

// Function to process incoming data and push to Firebase
void processData(String data) {
  // Example processing of the incoming data
  Serial.println("Processing data: " + data);

  // Parse and process the data as needed
  // Example: extracting sensor values from the received string
  // Assuming data format: "T:temperature,H:humidity,W:weight,S:soundLevel"
  int tempIndex = data.indexOf("T:");
  int humIndex = data.indexOf("H:");
  int weightIndex = data.indexOf("W:");
  int soundIndex = data.indexOf("S:");

  if (tempIndex != -1 && humIndex != -1 && weightIndex != -1 && soundIndex != -1) {
    String tempStr = data.substring(tempIndex + 2, humIndex - 1);
    String humStr = data.substring(humIndex + 2, weightIndex - 1);
    String weightStr = data.substring(weightIndex + 2, soundIndex - 1);
    String soundStr = data.substring(soundIndex + 2);

    float temperature = tempStr.toFloat();
    float humidity = humStr.toFloat();
    float weight = weightStr.toFloat();
    int soundLevel = soundStr.toInt();

    // Print parsed values
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.print(" %, Weight: ");
    Serial.print(weight);
    Serial.print(" kg, Sound Level: ");
    Serial.println(soundLevel);

    // Push data to Firebase
    if (Firebase.ready()) {
      String path = "/sensor_data";
      FirebaseJson json;
      json.set("temperature", temperature);
      json.set("humidity", humidity);
      json.set("weight", weight);
      json.set("soundLevel", soundLevel);
      if (Firebase.RTDB.setJSON(&fbdo, path, &json)) {
        Serial.println("Data pushed successfully");
      } else {
        Serial.println("Failed to push data: " + fbdo.errorReason());
      }
    }
  } else {
    Serial.println("Invalid data format.");
  }
}

