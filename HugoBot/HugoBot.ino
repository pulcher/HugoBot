
#include "MeMegaPi.h"
#include "Arduino.h"
#include "SoftwareSerial.h"
MePS2 MePS2(PORT_15);
void setup() {
  Serial.begin(115200);
  MePS2.begin(115200);
}

void loop() {
  MePS2.loop();
  
  if (MePS2.ButtonPressed(MeJOYSTICK_UP))
  {
    Serial.println("UP is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_DOWN))
  {
    Serial.println("DOWN is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_LEFT))
  {
    Serial.println("LEFT is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_RIGHT))
  {
    Serial.println("RIGHT is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_TRIANGLE))
  {
    Serial.println("TRIANGLE is pressed!");
  }
  if (MePS2.ButtonPressed(MeJOYSTICK_XSHAPED))
  {
    Serial.println("XSHAPED is pressed!");
  }
  if (MePS2.ButtonPressed(MeJOYSTICK_SQUARE))
  {
    Serial.println("SQUARE is pressed!");
  }
  if (MePS2.ButtonPressed(MeJOYSTICK_ROUND))
  {
    Serial.println("ROUND is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_START))
  {
    Serial.println("START is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_SELECT))
  {
    Serial.println("SELECT is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_BUTTON_L))
  {
    Serial.println("BUTTON_L is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_BUTTON_R))
  {
    Serial.println("BUTTON_R is pressed!");
  }
  
  handleJoystick(MeJOYSTICK_RX, MeJOYSTICK_RY);
}

void handleJoystick(uint8_t x, uint8_t y) {
  int xRead, yRead = 0;

  xRead = MePS2.MeAnalog(x);
  yRead = MePS2.MeAnalog(y);

  double radianAngle = atan2(yRead, xRead);
  double degreeAngle = radianAngle * 180/PI;

  if (abs(xRead) > 0 || abs(yRead) > 0) {
    Serial.print("Right joystick angle value is: ");
    Serial.print(degreeAngle);
    Serial.print(", ");
    Serial.println(radianAngle);
  }
  
}



