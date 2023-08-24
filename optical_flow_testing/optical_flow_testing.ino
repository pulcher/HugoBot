#include "Optical_Flow_Sensor.h"

// param #1 digital pin 10 for chip select
// param #2 sensor type either PAA5100 or PMW3901 
Optical_Flow_Sensor flow(10, PAA5100);

void setup() {
  Serial.begin(9600);

  if (!flow.begin()) {
    Serial.println("Initialization of the flow sensor failed");
    while(1) { }
  }
}

int16_t deltaX,deltaY;
int16_t x,y = 0;

void loop() {
  // Get motion count since last call
  flow.readMotionCount(&deltaX, &deltaY);

  x = x + deltaX;
  y = y + deltaY;
  
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(", Y: ");
  Serial.print(y);
  Serial.print("\n");

  delay(50);
}
