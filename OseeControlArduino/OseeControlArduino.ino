#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ArduinoJson.h>

// MAC address for your Ethernet shield – change as necessary.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Static IP address for the Arduino (adjust to your network)
IPAddress ip(192, 168, 10, 100);

// Osee Go Stream switcher IP and UDP ports (as given in the reference)
IPAddress switcherIP(192, 168, 10, 101);
const unsigned int outPort = 19018;
const unsigned int localPort = 19019;

// UDP instance
EthernetUDP Udp;

// Button pin definitions (first 4 buttons for pgm, next 4 for pvw)
const int pgmButtonPins[4] = {2, 3, 4, 5};
const int pvwButtonPins[4] = {6, 7, 8, 9};

// LED pin definitions (first 4 leds for pgm, next 4 for pvw)
const int pgmLedPins[4] = {10, 11, 12, 13};
const int pvwLedPins[4] = {22, 23, 24, 25};

// Debounce tracking for buttons
unsigned long lastDebounceTime[8] = {0};
const unsigned long debounceDelay = 50;  // milliseconds
bool lastButtonState[8] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};

void setup() {
  // Start serial for debug output
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB ports.
  }
  Serial.println("UDP Control Using GoStream Protocol");

  // Initialize button input pins with pull-up resistors.
  for (int i = 0; i < 4; i++) {
    pinMode(pgmButtonPins[i], INPUT_PULLUP);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(pvwButtonPins[i], INPUT_PULLUP);
  }

  // Initialize LED output pins.
  for (int i = 0; i < 4; i++) {
    pinMode(pgmLedPins[i], OUTPUT);
    digitalWrite(pgmLedPins[i], LOW);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(pvwLedPins[i], OUTPUT);
    digitalWrite(pvwLedPins[i], LOW);
  }

  // Start Ethernet and UDP.
  Ethernet.begin(mac, ip);
  Udp.begin(localPort);
  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Listening on UDP port ");
  Serial.println(localPort);
}

void loop() {
  // Check buttons for pgm and pvw commands.
  checkButtons();

  // Listen for incoming UDP packets.
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    char packetBuffer[256];
    int len = Udp.read(packetBuffer, sizeof(packetBuffer) - 1);
    if (len > 0) {
      packetBuffer[len] = '\0'; // Null-terminate the buffer
    }
    Serial.print("Received packet: ");
    Serial.println(packetBuffer);

    // Process the incoming JSON message to update LED states.
    processIncomingJson(packetBuffer);
  }
}

// Function to check button states and send UDP message on button press.
void checkButtons() {
  // Process pgm buttons (indexes 0-3)
  for (int i = 0; i < 4; i++) {
    bool reading = digitalRead(pgmButtonPins[i]);
    int index = i;  // overall index 0-3 for pgm buttons

    if (reading != lastButtonState[index]) {
      lastDebounceTime[index] = millis();
      lastButtonState[index] = reading;
    }

    if ((millis() - lastDebounceTime[index]) > debounceDelay && reading == LOW) {
      // Button pressed: send pgm get request.
      sendUdpCommand("pgmTally", "get");
      delay(200); // simple delay to avoid multiple triggers
    }
  }

  // Process pvw buttons (indexes 4-7)
  for (int i = 0; i < 4; i++) {
    bool reading = digitalRead(pvwButtonPins[i]);
    int index = i + 4;  // overall index for pvw buttons

    if (reading != lastButtonState[index]) {
      lastDebounceTime[index] = millis();
      lastButtonState[index] = reading;
    }

    if ((millis() - lastDebounceTime[index]) > debounceDelay && reading == LOW) {
      // Button pressed: send pvw get request.
      sendUdpCommand("pvwTally", "get");
      delay(200); // simple delay to avoid multiple triggers
    }
  }
}

// Function to send a UDP command in JSON format.
void sendUdpCommand(const char* id, const char* type) {
  // Construct JSON command. In this reference, "get" command uses an empty "value" array.
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

// Function to process incoming JSON and update LED states based on response.
void processIncomingJson(const char* jsonData) {
  // Allocate a JSON document
  // Use a capacity that fits your expected JSON payload.
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.f_str());
    return;
  }

  const char* id = doc["id"];
  const char* type = doc["type"];

  // We expect the "value" field to be an array of 4 numbers.
  JsonArray values = doc["value"].as<JsonArray>();
  if (!values.isNull() && values.size() >= 4) {
    if (strcmp(id, "pgmTally") == 0) {
      // Update pgm LEDs.
      for (int i = 0; i < 4; i++) {
        int stateValue = values[i]; // expected state, e.g., 0 for off, non-zero for on
        digitalWrite(pgmLedPins[i], stateValue != 0 ? HIGH : LOW);
      }
      Serial.println("Updated PGM LEDs.");
    } else if (strcmp(id, "pvwTally") == 0) {
      // Update pvw LEDs.
      for (int i = 0; i < 4; i++) {
        int stateValue = values[i];
        digitalWrite(pvwLedPins[i], stateValue != 0 ? HIGH : LOW);
      }
      Serial.println("Updated PVW LEDs.");
    }
  }
}
