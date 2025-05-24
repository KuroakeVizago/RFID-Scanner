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
  setTextLCD("Scan a card ^-^");

  // Reinit the RFID Scanner
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

  cardUID.toUpperCase();  // Capitalize UID
  setTextLCD("UID:" + cardUID);
  
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  // Delay per scan
  delay(500);
}

void setTextLCD(String text) {
  lcd.init();          // Initialize LCD

  lcd.clear();             // Clear the screen
  lcd.setCursor(0, 0);     // First line

  if (text.length() <= 16) {
    lcd.print(text);       // If short, print on first line
  } else {
    // Split across two lines
    lcd.print(text.substring(0, 16));
    lcd.setCursor(0, 1);   // Second line
    lcd.print(text.substring(16, min((unsigned int)32, text.length())));
  }

  lcd.setCursor(0, 2);     // Third line
}


void beep() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn buzzer on
  delay(100);                     // Duration of beep
  digitalWrite(BUZZER_PIN, LOW);  // Turn buzzer off
}
