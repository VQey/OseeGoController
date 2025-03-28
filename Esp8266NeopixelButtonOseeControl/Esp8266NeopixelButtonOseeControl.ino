#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// WiFi network credentials
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// UDP Port assignment and  Osee Go Stream switcher IP
const IPAddress switcherIP(192, 168, 10, 101);
const unsigned int outPort = 19018;
const unsigned int localPort = 19019;

// Create UDP instance
WiFiUDP Udp;

// NeoPixel configuration: 8 LEDs on pin 0
#define LED_PIN 0
#define NUM_LEDS 8
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Button pin definitions
// First 4 buttons for pgm, next 4 for pvw.
const int pgmButtonPins[4] = {16, 5, 4, 2};
const int pvwButtonPins[4] = {12, 14, 13, 15};

// Debounce variables for all 8 buttons
unsigned long lastDebounceTime[8] = {0};
const unsigned long debounceDelay = 50; // in milliseconds
bool lastButtonState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("ESP8266 UDP Control Using GoStream Protocol");

  // Initialize button pins as inputs with internal pull-ups.
  for (int i = 0; i < 4; i++) {
    pinMode(pgmButtonPins[i], INPUT_PULLUP);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(pvwButtonPins[i], INPUT_PULLUP);
  }

  // Initialize the NeoPixel strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  Udp.begin(localPort);
  Serial.print("Listening on UDP port ");
  Serial.println(localPort);
}

void loop() {
  // Check button states for sending UDP commands.
  checkButtons();
  
  // Check for incoming UDP packets.
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[256];
    int len = Udp.read(packetBuffer, sizeof(packetBuffer)-1);
    if (len > 0) {
      packetBuffer[len] = 0; // null-terminate the string
    }
    Serial.print("Received packet: ");
    Serial.println(packetBuffer);
    processIncomingJson(packetBuffer);
  }
}

// Check all buttons and send UDP commands when pressed.
void checkButtons() {
  // Process pgm buttons (indexes 0-3)
  for (int i = 0; i < 4; i++) {
    bool reading = digitalRead(pgmButtonPins[i]);
    int index = i; // indexes 0 to 3 for pgm buttons

    if (reading != lastButtonState[index]) {
      lastDebounceTime[index] = millis();
      lastButtonState[index] = reading;
    }

    if ((millis() - lastDebounceTime[index]) > debounceDelay && reading == LOW) {
      // Button pressed: send pgm get command.
      sendUdpCommand("pgmTally", "get");
      delay(200); // simple delay to avoid rapid multiple triggers
    }
  }
  
  // Process pvw buttons (indexes 4-7)
  for (int i = 0; i < 4; i++) {
    bool reading = digitalRead(pvwButtonPins[i]);
    int index = i + 4; // indexes 4 to 7 for pvw buttons

    if (reading != lastButtonState[index]) {
      lastDebounceTime[index] = millis();
      lastButtonState[index] = reading;
    }

    if ((millis() - lastDebounceTime[index]) > debounceDelay && reading == LOW) {
      // Button pressed: send pvw get command.
      sendUdpCommand("pvwTally", "get");
      delay(200); // simple delay to avoid rapid multiple triggers
    }
  }
}

// Function to send UDP command with a JSON payload.
void sendUdpCommand(const char* id, const char* type) {
  // Construct JSON command
  String jsonString = "{\"id\":\"";
  jsonString += id;
  jsonString += "\",\"type\":\"";
  jsonString += type;
  jsonString += "\",\"value\":[]}";
  
  Serial.print("Sending command: ");
  Serial.println(jsonString);

  Udp.beginPacket(switcherIP, outPort);
  Udp.write(jsonString.c_str());
  Udp.endPacket();
}

// Function to process incoming JSON data and update the NeoPixel LEDs.
void processIncomingJson(const char* jsonData) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.f_str());
    return;
  }

  const char* id = doc["id"];
  const char* type = doc["type"];
  
  JsonArray values = doc["value"].as<JsonArray>();
  if (!values.isNull() && values.size() >= 4) {
    if (strcmp(id, "pgmTally") == 0) {
      // Update first 4 LEDs for PGM status: use red color for ON.
      for (int i = 0; i < 4; i++) {
        int stateValue = values[i];
        if (stateValue != 0) {
          strip.setPixelColor(i, strip.Color(255, 0, 0)); // red on
        } else {
          strip.setPixelColor(i, 0); // off
        }
      }
      Serial.println("Updated PGM LEDs.");
    } else if (strcmp(id, "pvwTally") == 0) {
      // Update next 4 LEDs for PVW status: use blue color for ON.
      for (int i = 0; i < 4; i++) {
        int stateValue = values[i];
        if (stateValue != 0) {
          strip.setPixelColor(i + 4, strip.Color(0, 0, 255)); // blue on
        } else {
          strip.setPixelColor(i + 4, 0); // off
        }
      }
      Serial.println("Updated PVW LEDs.");
    }
    strip.show();
  }
}
