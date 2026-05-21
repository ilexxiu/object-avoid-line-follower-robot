#include <AFMotor.h>

#define irLeft A3
#define irRight A0

AF_DCMotor motor1(1, MOTOR12_1KHZ);
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);

int robotSpeed = 120;

void setup() {
  pinMode(irLeft, INPUT);
  pinMode(irRight, INPUT);

  motor1.setSpeed(robotSpeed);
  motor2.setSpeed(robotSpeed);
  motor3.setSpeed(robotSpeed);
  motor4.setSpeed(robotSpeed);

  Serial.begin(9600);
}

void loop() {

  int leftState = digitalRead(irLeft);
  int rightState = digitalRead(irRight);

  // FORWARD
  if (leftState == 0 && rightState == 0) {

    moveForward();

  }

  // TURN RIGHT
  else if (leftState == 0 && rightState == 1) {

    moveRight();
    delay(120);   // IMPORTANT

  }

  // TURN LEFT
  else if (leftState == 1 && rightState == 0) {

    moveLeft();
    delay(120);   // IMPORTANT

  }

  // BOTH WHITE
  else {

    // instead of stopping instantly
    moveForward();
    delay(80);

  }
}


// ================= MOVEMENTS =================

void moveForward() {

  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

void moveRight() {

  motor1.run(FORWARD);
  motor2.run(FORWARD);

  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
}

void moveLeft() {

  motor1.run(BACKWARD);
  motor2.run(BACKWARD);

  motor3.run(FORWARD);
  motor4.run(FORWARD);
}