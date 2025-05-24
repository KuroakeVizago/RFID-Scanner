#include <WiFi.h>

const char* ssid = "Moshi-Mosh-2";
const char* password = "12345678";

// GitHub host and HTTPS port
const char* host = "guthib.com";
const int httpsPort = 443;

// GitHub uses a valid certificate, so we can skip certificate validation (for demo purposes)
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.println("Connecting to Guthib...");

  // Allow insecure connection (⚠️ NOT RECOMMENDED FOR PRODUCTION)
  client.setInsecure();

  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection to GitHub failed!");
    return;
  }

  // Send HTTP GET request
  client.println("GET / HTTP/1.1");
  client.println("Host: guthib.com");
  client.println("Connection: close");
  client.println(); // Empty line ends the headers

  // Wait for response
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break; // Headers received
  }

  Serial.println("GitHub Homepage:");
  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }
}

void loop() {
  // Nothing here
}