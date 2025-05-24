#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 5      // SDA to D9
#define RST_PIN 22    // RST to D5
#define BUZZER_PIN 26 // VCC of buzzer to D3 (GPIO 17)

MFRC522 mfrc522(SS_PIN, RST_PIN);

// Set the LCD I2C address to 0x27 or 0x3F depending on your module
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is most common

enum APP_STATE_ENUM {
  WAITING_CARD,
  CARD_DETECTED
};

void setup() {
  Serial.begin(115200);

  lcd.init();          // Initialize LCD
  lcd.setBacklight(HIGH);

  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  mfrc522.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Keep buzzer off initially

  Serial.println("Place an RFID tag near the reader...");
}

void loop() {

  SPI.begin(18, 19, 23, 5); // SCK, MISO, MOSI, SS
  mfrc522.PCD_Init();

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Beep when card is scanned
  beep();

  String cardUID = "";

  // Show UID
  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);

    if (mfrc522.uid.uidByte[i] < 0x10) {
      cardUID += "0";  // Add leading zero
    }
    cardUID += String(mfrc522.uid.uidByte[i], HEX);
  }

  lcd.init();          // Initialize LCD
  lcd.setBacklight(HIGH);
  lcd.setCursor(0, 0); // Column 0, Row 0
  lcd.print(cardUID);
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(100);
}

void beep() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn buzzer on
  delay(100);                     // Duration of beep
  digitalWrite(BUZZER_PIN, LOW);  // Turn buzzer off
}
