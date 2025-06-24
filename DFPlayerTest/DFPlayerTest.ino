#include <DFRobotDFPlayerMini.h>

#define RXD2 16  // Connect to TX of DFPlayer
#define TXD2 17  // Connect to RX of DFPlayer

DFRobotDFPlayerMini myDFPlayer;
HardwareSerial mySerial(2); // Use Serial2 (HSPI UART) on ESP32

void setup()
{
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("Initializing DFPlayer...");

  if (!myDFPlayer.begin(mySerial)) {
    Serial.println("Unable to begin DFPlayer Mini:");
    Serial.println("1. Check connections");
    Serial.println("2. Insert SD card");
    while (true);
  }

  Serial.println("DFPlayer Mini online.");
  myDFPlayer.volume(10);  // Set volume (0 to 30)
  myDFPlayer.play(1);     // Play first file (0001.mp3)
}

void loop()
{
  // Nothing here unless you want to trigger more sounds
}
