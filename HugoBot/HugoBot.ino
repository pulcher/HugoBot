
#include "MeMegaPi.h"
#include "Arduino.h"
#include "SoftwareSerial.h"
#include <ArduinoJson.h>
#include "directions.h"

#define NUM_MOTORS 4
#define MOTOR_LF 0
#define MOTOR_RF 1
#define MOTOR_RR 3
#define MOTOR_LR 2

// Multipliers for direction "correction"
#define FORWARD            1
#define REVERSE  (int8_t)-1
#define STOP               0

#define WHEELS_FRONT 0    // Lookup index for the FRONT wheels, in the "directions" array
#define WHEELS_REAR  1    // Lookup index for the REAR wheels, in the "directions" array

#define WHEELS_LEFT  0    // Lookup index for the LEFT wheel in the inner "directions" array
#define WHEELS_RIGHT 1    // Lookup index for the REAR wheel in the inner "directions" array

const int8_t Front_Wheels[]    = {FORWARD, REVERSE};
const int8_t Rear_Wheels[]     = {FORWARD, REVERSE};
const int8_t All_Wheels[][2]   = {{REVERSE, FORWARD}, {REVERSE, FORWARD}};

const int8_t Directions [12][2][2] = {
                                    { {STOP, FORWARD},     {FORWARD, STOP}},       {{FORWARD, FORWARD},  {FORWARD, FORWARD}},    {{FORWARD, STOP}, {STOP, FORWARD}       },
                                    { {REVERSE, FORWARD},  {FORWARD, REVERSE}},    {{STOP, STOP},        {STOP, STOP}},          {{FORWARD, REVERSE}, {REVERSE, FORWARD} },
                                    { {REVERSE, STOP},  {STOP, REVERSE}},          {{REVERSE, REVERSE},  {REVERSE, REVERSE}},    {{STOP, REVERSE}, {REVERSE, STOP} },
                                    { {REVERSE, FORWARD},  {REVERSE, FORWARD}},    {{STOP, STOP},        {STOP, STOP}},          {{FORWARD, REVERSE}, {FORWARD, REVERSE} }
                                 };

MePS2 MePS2(PORT_15);

MeMegaPiDCMotor motor0(PORT1A);
MeMegaPiDCMotor motor1(PORT1B);
MeMegaPiDCMotor motor2(PORT2A);
MeMegaPiDCMotor motor3(PORT2B);

Servo clawServo;

MeMegaPiDCMotor motors[] = {motor0, motor1, motor2, motor3};

int8_t motors_pwm[NUM_MOTORS];
int8_t motors_pwm_previous[NUM_MOTORS];

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  MePS2.begin(115200);

  for(int i = 0; i < NUM_MOTORS; i++ ) {
    motors_pwm[i] = motors_pwm_previous[i] = 0;
  }

  clawServo.attach(60);
  clawServo.writeMicroseconds(1200);

}

bool buttonPressed = false;
bool isAutonomous = false;
JsonDocument doc;

void loop() {
  
  MePS2.loop();

  if (MePS2.ButtonPressed(MeJOYSTICK_SELECT))
  {
    isAutonomous = !isAutonomous;
    Serial.print("isAutonomous: ");
    Serial.println(isAutonomous);

    delay(100);
  }

  if (isAutonomous) {
    handleSerial2();
  } else {
  
  if (MePS2.ButtonPressed(MeJOYSTICK_UP))
  {
    Serial.println("UP is pressed!");

    moveBot(DIRECTION_F, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_DOWN))
  {
    Serial.println("DOWN is pressed!");

    moveBot(DIRECTION_R, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_LEFT))
  {
    Serial.println("LEFT is pressed!");
        
    moveBot(DIRECTION_SL, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_RIGHT))
  {
    Serial.println("RIGHT is pressed!");

    moveBot(DIRECTION_SR, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_TRIANGLE))
  {
    Serial.println("TRIANGLE(1) is pressed!");

    moveBot(DIRECTION_ROTL, 100);
    buttonPressed = true;
  }
  // if (MePS2.ButtonPressed(MeJOYSTICK_XSHAPED))
  // {
  //   Serial.println("XSHAPED is pressed!");
  // }
  if (MePS2.ButtonPressed(MeJOYSTICK_SQUARE))
  {
    Serial.println("SQUARE(4) is pressed!");

    moveBot(DIRECTION_ROTR, 100);
    buttonPressed = true;
  }
  // if (MePS2.ButtonPressed(MeJOYSTICK_ROUND))
  // {
  //   Serial.println("ROUND is pressed!");
  // }

  // if (MePS2.ButtonPressed(MeJOYSTICK_START))
  // {
  //   Serial.println("START is pressed!");
  // }

  if (MePS2.ButtonPressed(MeJOYSTICK_L1))
  {
    Serial.println("BUTTON_L1 is pressed!");

    moveBot(DIRECTION_FL, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_L2))
  {
    Serial.println("BUTTON_L2 is pressed!");

    moveBot(DIRECTION_RL, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_R1))
  {
    Serial.println("BUTTON_R1 is pressed!");

    moveBot(DIRECTION_FR, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_R2))
  {
    Serial.println("BUTTON_R2 is pressed!");

    moveBot(DIRECTION_RR, 100);
    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_BUTTON_R))
  {
    Serial.println("BUTTON_R is pressed!");
    clawOpen();
  }
  
  if (MePS2.ButtonPressed(MeJOYSTICK_BUTTON_L))
  {
    Serial.println("BUTTON_L is pressed!");
    clawClosed();
  }


  handleJoystick(MeJOYSTICK_RX, MeJOYSTICK_RY);
  }
  if (!buttonPressed && !isAutonomous) {
    moveBot(DIRECTION_NONE, 0);
  }

  // read the serial in for json that looks like
  // TheHugoDahl: Like this(ish): {"operation": "move", "direction": "N", "timeMS": 150}
  buttonPressed = false;
}

void handleJoystick(uint8_t x, uint8_t y) {
  int xRead, yRead = 0;

  xRead = MePS2.MeAnalog(x);
  yRead = MePS2.MeAnalog(y);

  double radianAngle = atan2(yRead, xRead);
  double degreeAngle = radianAngle * 180/PI;

  if (abs(xRead) > 0 || abs(yRead) > 0) {
    // Serial.print("Right joystick angle value is: ");
    // Serial.print(degreeAngle);
    // Serial.print(", ");
    // Serial.println(radianAngle);
  }
  
  delay(100);
}

void handleSerial2() {
  if (Serial2.available()) {
    String received_data = Serial2.readStringUntil('\n');  // Read data from Serial2
    DeserializationError error = deserializeJson(doc, received_data);
    String operation = doc["operation"];
    Serial.println("Received: operation: " + operation);

    if (operation == "move") {
      uint8_t direction = doc["data"]["direction"];
      uint8_t speed = doc["data"]["speed"];

      Serial.print("direction: ");
      Serial.print(direction);
      Serial.print(", Speed: ");
      Serial.print(speed);
      moveBot(direction, speed);
    }
  }


}

void moveBot(uint8_t botDirection, uint16_t speed) {
  auto direction = Directions[botDirection];
  // Serial.print("botDirection: ");
  // Serial.print(botDirection);
  // Serial.print(", ");
  // Serial.print("direction[WHEELS_FRONT][WHEELS_LEFT]: ");
  // Serial.print(direction[WHEELS_FRONT][WHEELS_LEFT]);
  // Serial.print(", ");
  // Serial.print("All_Wheels[WHEELS_FRONT][WHEELS_LEFT]: ");
  // Serial.print(All_Wheels[WHEELS_FRONT][WHEELS_LEFT]);
  // Serial.println("");

  setWheelSpeed(MOTOR_LF, direction[WHEELS_FRONT][WHEELS_LEFT]  * All_Wheels[WHEELS_FRONT][WHEELS_LEFT]  * speed);
  setWheelSpeed(MOTOR_RF, direction[WHEELS_FRONT][WHEELS_RIGHT] * All_Wheels[WHEELS_FRONT][WHEELS_RIGHT] * speed);
  setWheelSpeed(MOTOR_LR, direction[WHEELS_REAR][WHEELS_LEFT]   * All_Wheels[WHEELS_REAR][WHEELS_LEFT]   * speed);
  setWheelSpeed(MOTOR_RR, direction[WHEELS_REAR][WHEELS_RIGHT]  * All_Wheels[WHEELS_REAR][WHEELS_RIGHT]  * speed);
}

void setWheelSpeed(int8_t motorIdx, int16_t speed) {
  motors[motorIdx].run(speed);
}

void clawOpen() {
  setClaw(1300);
}

void clawClosed() {
  setClaw(2000);
}

void setClaw(uint16_t pwmSetting) {
  clawServo.writeMicroseconds(pwmSetting);

  delay(10);
}