// Teensy 4.1 Serial 2 "blah" output example
#include <ArduinoJson.h>

StaticJsonDocument<200> doc;


void setup() {
  Serial.begin(9600);

  // Initialize Serial 2 with a baud rate of 9600
  Serial8.begin(115200, SERIAL_8N1);

    // Add values in the document
  //
  doc["sensor"] = "gps";
  doc["time"] = 1351824120;

  // Add an array.
  //
  JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);

}

//char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

void loop() {
  Serial.println("blah");
  // Send "blah" to Serial 2
  //Serial8.println("blah");

  serializeJson(doc, Serial8);
  Serial8.println();

  delay(50); // Wait for 1 second
}
