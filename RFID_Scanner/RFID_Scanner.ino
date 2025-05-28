#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>

const char* ssid = "Yasapintar";
const char* password = "tuban456";

const String DEVICE_SERIAL_NUMBER = "dvc-001";
const String API_ENDPOINT = "https://4e52-180-247-94-40.ngrok-free.app/api";

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

// Timing
unsigned long previousMillis = 0;
const long clockUpdateInterval = 1000;  // update every 1 second

const int buttonPin = 2;  // D4 on Wemos D1 R32 (GPIO2)

enum APP_BUTTON_STATE_ENUM {
  PRESSED,
  RELEASED,
};

APP_BUTTON_STATE_ENUM appButtonStateCurrent = RELEASED;

enum ABSEN_MODE {
  MASUK,
  PULANG,
};

ABSEN_MODE absenState = MASUK;

enum APP_STATE_ENUM {
  INIT_STATE_ABSEN,
  WAITING_CARD_ABSEN,
  CARD_DETECTED_ABSEN,

  INIT_STATE_ADD,
  WAITING_CARD_ADD,
  CARD_DETECTED_ADD,

  INIT_STATE_BOT,
  WAITING_CARD_BOT,
  CARD_DETECTED_BOT,
};

APP_STATE_ENUM appStateCurrent = INIT_STATE_ABSEN;

void setup() {
  // INIT Button
  pinMode(buttonPin, INPUT_PULLUP);  // Button uses internal pull-up

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

  appConnectWifi();

  // Initialize time from NTP
  configTzTime("WIB-7", "id.pool.ntp.org", "asia.pool.ntp.org");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Time initialized");

  // Init SPI and RFID
  SPI.begin();  // uses default SPI pins
  rfid.PCD_Init();
  delay(100);
  Serial.println("RFID reader ready");
}

void loop() {
  listenButton();

  switch (appStateCurrent) {
    case INIT_STATE_ABSEN:
      appInitStateAbsen();
      break;
    case WAITING_CARD_ABSEN:
      appWaitingCardAbsen();
      break;
    case CARD_DETECTED_ABSEN:
      appCardDetectedAbsen();
      break;
    case INIT_STATE_ADD:
      appInitStateAdd();
      break;
    case WAITING_CARD_ADD:
      appWaitingCardAdd();
      break;
    case CARD_DETECTED_ADD:
      appCardDetectedAdd();
      break;
    case INIT_STATE_BOT:
      appInitStateBot();
      break;
    case WAITING_CARD_BOT:
      appWaitingCardBot();
      break;
    case CARD_DETECTED_BOT:
      appCardDetectedBot();
      break;
  }
}

void beep() {
  digitalWrite(BUZZER_PIN, HIGH);  // Turn buzzer on
  delay(100);                      // Duration of beep
  digitalWrite(BUZZER_PIN, LOW);   // Turn buzzer off
}

void appInitStateAdd() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Registrasi Kartu");

  lcd.setCursor(0, 1);
  lcd.print("Tempel Kartu...");

  appStateCurrent = WAITING_CARD_ADD;
}

void appInitStateBot() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Telegram Bot");

  lcd.setCursor(0, 1);
  lcd.print("Tempel Kartu...");

  appStateCurrent = WAITING_CARD_BOT;
}


void appWaitingCardAbsen() {
  // Update clock every second
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= clockUpdateInterval) {
    previousMillis = currentMillis;

    // Display clock
    lcd.setCursor(0, 0);
    lcd.print(getTimeStr());
  }

  lcd.setCursor(9, 0);
  if (absenState == MASUK) {
    lcd.print("MASUK ");
  }
  if (absenState == PULANG) {
    lcd.print("PULANG");
  }

  // Look for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  appStateCurrent = CARD_DETECTED_ABSEN;
}

void appCardDetectedAbsen() {
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

  appStateCurrent = INIT_STATE_ABSEN;

  // Halt and cleanup
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void appCardDetectedAdd() {
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

  HTTPClient http;

  String url = API_ENDPOINT + "/device/register-kartu?serial=" + DEVICE_SERIAL_NUMBER + "&no_kartu=" + uid;  // Ganti dengan URL kamu
  Serial.println(url);
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Response:");
    Serial.println(payload);
  } else {
    Serial.print("Error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();

  appStateCurrent = INIT_STATE_ADD;

  // Halt and cleanup
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void appCardDetectedBot() {
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

  appStateCurrent = INIT_STATE_BOT;

  // Halt and cleanup
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void appWaitingCardAdd() {
  // Look for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  appStateCurrent = CARD_DETECTED_ADD;
}

void appWaitingCardBot() {
  // Look for new RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  appStateCurrent = CARD_DETECTED_BOT;
}

void appInitStateAbsen() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Tempel Kartu...");

  appStateCurrent = WAITING_CARD_ABSEN;
}

void appConnectWifi() {
  lcd.clear();


  lcd.setCursor(0, 0);
  lcd.print("Connecting to");

  lcd.setCursor(0, 1);
  lcd.print("WiFi.....");

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");

  delay(2000);
}

// Function to get current time as string
String getTimeStr() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "No Time";
  }
  char buffer[9];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  return String(buffer);
}

void listenButton() {

  int reading = digitalRead(buttonPin);

  if (reading == 1 && appButtonStateCurrent == RELEASED) {
    Serial.println("NYALA");
    appButtonStateCurrent = PRESSED;


    if (appStateCurrent == WAITING_CARD_ABSEN && absenState == MASUK) {
      absenState = PULANG;
    } else if (appStateCurrent == WAITING_CARD_ABSEN && absenState == PULANG) {
      appStateCurrent = INIT_STATE_ADD;
    } else if (appStateCurrent == WAITING_CARD_ADD && absenState == PULANG) {
      appStateCurrent = INIT_STATE_BOT;
    } else if (appStateCurrent == WAITING_CARD_BOT) {
      appStateCurrent = INIT_STATE_ABSEN;
      absenState = MASUK;
    }

    Serial.println(appStateCurrent);

    beep();
  }

  if (reading == 0 && appButtonStateCurrent == PRESSED) {
    Serial.println("MATI");
    appButtonStateCurrent = RELEASED;
  }
}