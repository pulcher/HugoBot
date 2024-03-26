#include <Arduino.h>
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;

void setup() {
    Serial2.begin(9600);  // Initialize Serial1 (TX1: pin 19, RX1: pin 18)
    Serial.begin(115200);   // Initialize Serial for debugging (optional)
}

void loop() {
    if (Serial2.available()) {
        String received_data = Serial2.readStringUntil('\n');  // Read data from Serial2
        DeserializationError error = deserializeJson(doc, received_data);
        long jTime = doc["time"];
        Serial.println("Received message from Arduino 1: " + received_data + " at " + jTime);
    }
}
