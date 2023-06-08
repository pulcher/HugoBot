
#include "MeMegaPi.h"
#include "Arduino.h"
#include "SoftwareSerial.h"

#define NUM_MOTORS 4
#define MOTOR_LF 0
#define MOTOR_RF 1
#define MOTOR_RR 2
#define MOTOR_LR 3

MePS2 MePS2(PORT_15);

MeMegaPiDCMotor motor0(PORT1A);
MeMegaPiDCMotor motor1(PORT1B);
MeMegaPiDCMotor motor2(PORT2A);
MeMegaPiDCMotor motor3(PORT2B);

MeMegaPiDCMotor motors[NUM_MOTORS];

int8_t motors_pwm[NUM_MOTORS];
int8_t motors_pwm_previous[NUM_MOTORS];

void setup() {
  Serial.begin(115200);
  MePS2.begin(115200);

  for(int i = 0; i < NUM_MOTORS; i++ ) {
    motors_pwm[i] = motors_pwm_previous[i] = 0;
  }

}

bool buttonPressed = false;

void loop() {
  MePS2.loop();
  
  if (MePS2.ButtonPressed(MeJOYSTICK_UP))
  {
    Serial.println("UP is pressed!");

    motors_pwm[MOTOR_LF] = 100;
    motors_pwm[MOTOR_RF] = 0;
    motors_pwm[MOTOR_RR] = 0;
    motors_pwm[MOTOR_LR] = 0;

    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_DOWN))
  {
    Serial.println("DOWN is pressed!");

    motors_pwm[MOTOR_LF] = -100;
    motors_pwm[MOTOR_RF] = -100;
    motors_pwm[MOTOR_RR] = -100;
    motors_pwm[MOTOR_LR] = -100;

    buttonPressed = true;
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_LEFT))
  {
    Serial.println("LEFT is pressed!");
  }

  if (MePS2.ButtonPressed(MeJOYSTICK_RIGHT))
  {
    Serial.println("RIGHT is pressed!");
  }

  // if (MePS2.ButtonPressed(MeJOYSTICK_TRIANGLE))
  // {
  //   Serial.println("TRIANGLE is pressed!");
  // }
  // if (MePS2.ButtonPressed(MeJOYSTICK_XSHAPED))
  // {
  //   Serial.println("XSHAPED is pressed!");
  // }
  // if (MePS2.ButtonPressed(MeJOYSTICK_SQUARE))
  // {
  //   Serial.println("SQUARE is pressed!");
  // }
  // if (MePS2.ButtonPressed(MeJOYSTICK_ROUND))
  // {
  //   Serial.println("ROUND is pressed!");
  // }

  // if (MePS2.ButtonPressed(MeJOYSTICK_START))
  // {
  //   Serial.println("START is pressed!");
  // }

  // if (MePS2.ButtonPressed(MeJOYSTICK_SELECT))
  // {
  //   Serial.println("SELECT is pressed!");
  // }

  // if (MePS2.ButtonPressed(MeJOYSTICK_BUTTON_L))
  // {
  //   Serial.println("BUTTON_L is pressed!");
  // }

  // if (MePS2.ButtonPressed(MeJOYSTICK_BUTTON_R))
  // {
  //   Serial.println("BUTTON_R is pressed!");
  // }
  
  handleJoystick(MeJOYSTICK_RX, MeJOYSTICK_RY);

  if (!buttonPressed) {
    motors_pwm[MOTOR_LF] = 0;
    motors_pwm[MOTOR_RF] = 0;
    motors_pwm[MOTOR_RR] = 0;
    motors_pwm[MOTOR_LR] = 0;
  }
  
  handleMotors();

  buttonPressed = false;
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

void handleMotors() {
  for(int i = 0; i < NUM_MOTORS; i++) {
    if (motors_pwm[i] != motors_pwm_previous[i]) {
      motors_pwm_previous[i] = motors_pwm[i];
    }
  }

  motor0.run(motors_pwm[0]);
  motor1.run(motors_pwm[1]);
  motor2.run(motors_pwm[2]);
  motor3.run(motors_pwm[3]);
}

