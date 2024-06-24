#include <SPI.h>
#include <RH_RF95.h>

// Singleton instance of the radio driver
RH_RF95 rf95;

// Change to match the hardware
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2

// Frequency of LoRa module
#define RF95_FREQ 915.0

void setup() 
{
  // Manual reset
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.println("RFM95 LoRa Receiver");
  
  // Initialize the RFM95
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  
  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }
  
  // Set frequency
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Set transmitter power
  rf95.setTxPower(23, false);
}

void loop() 
{
  if (rf95.available()) {
    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (rf95.recv(buf, &len)) {
      buf[len] = '\0'; // Null-terminate the received data
      
      Serial.print("Received: ");
      Serial.println((char*)buf);

      // Extract the transmitter ID from the received message
      int transmitterID = -1;
      sscanf((char*)buf, "ID: %d", &transmitterID);

      if (transmitterID != -1) {
        Serial.print("Transmitter ID: ");
        Serial.println(transmitterID);
        
        // Further processing based on transmitter ID
        // For example, store data or perform specific actions
      } else {
        Serial.println("Failed to parse transmitter ID");
      }
      
      // Blink the LED to show a successful message received
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    } else {
      Serial.println("Receive failed");
    }
  }
}
