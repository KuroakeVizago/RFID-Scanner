#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

// Define RC522 pins
#define SS_PIN 5   // GPIO5 (D5)
#define RST_PIN 4  // GPIO4 (D4)

// Define I2C pins (optional, ESP32 allows this)
#define I2C_SDA 21  // GPIO21 (D21)
#define I2C_SCL 22  // GPIO22 (D22)

#define BUZZER_PIN 26  // VCC of buzzer to D3 (GPIO 17)

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Check your LCD I2C address if not 0x27

String uid = "";

enum APP_STATE_ENUM {
  INIT_STATE,
  WAITING_CARD_ABSEN,
  CARD_DETECTED_ABSEN,
};

APP_STATE_ENUM appStateCurrent = INIT_STATE;

void setup() {
  // Init buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Keep buzzer off initially

  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(100);

  // Init I2C LCD
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();

  // Init SPI and RFID
  SPI.begin();  // uses default SPI pins
  rfid.PCD_Init();
  delay(100);
  Serial.println("RFID reader ready");
}

void loop() {
  if (appStateCurrent == INIT_STATE) {
    appStandby();
    appStateCurrent = WAITING_CARD_ABSEN;
  }

  if (appStateCurrent == WAITING_CARD_ABSEN) {
    // Look for new RFID card
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
      return;
    }

    appStateCurrent = CARD_DETECTED_ABSEN;
  }

  if (appStateCurrent == CARD_DETECTED_ABSEN) {
    uid = "";
    // Build UID string
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) uid += "0";
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    uid.toUpperCase();

    // Print to Serial
    Serial.println("Card UID: " + uid);

    // Display on LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kartu Terdeteksi:");
    lcd.setCursor(0, 1);
    lcd.print(uid);

    beep();

    delay(3000);  // Hold message

    appStateCurrent = INIT_STATE;

    // Halt and cleanup
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
}

void beep() {
  digitalWrite(BUZZER_PIN, HIGH);  // Turn buzzer on
  delay(100);                      // Duration of beep
  digitalWrite(BUZZER_PIN, LOW);   // Turn buzzer off
}

void appStandby() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tmpl Kartu Absen");
}
