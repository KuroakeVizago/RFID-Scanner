#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD I2C address to 0x27 or 0x3F depending on your module
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is most common

void setup() {
  Serial.begin(115200);

  lcd.init();          // Initialize LCD
  lcd.backlight();     // Turn on backlight
  lcd.setCursor(0, 0); // Column 0, Row 0
  lcd.print("Hello, world!");
  lcd.setCursor(0, 1); // Column 0, Row 1
  lcd.print("ESP32 + LCD");
  lcd.setCursor(0, 2); // Delete the cursor
}

void loop() {
  // Nothing here
}
