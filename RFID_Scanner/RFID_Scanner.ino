#include <SPI.h>
#include <MFRC522.h>

// Pin definitions (correspond to D9 and D5 on WEMOS D1 R32)
#define SS_PIN 5    // SDA connected to D9 (GPIO 5)
#define RST_PIN 22  // RST connected to D5 (GPIO 22)

// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);       // Initialize serial communications
  SPI.begin(18, 19, 23, 5);   // SCK=18 (D8), MISO=19 (D6), MOSI=23 (D7), SS=5 (D9)
  mfrc522.PCD_Init();         // Initialize RFID reader

  Serial.println("Place an RFID tag near the reader...");
}

void loop() {
  // Look for new RFID cards
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Show UID on serial monitor
  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  // Halt communication
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
