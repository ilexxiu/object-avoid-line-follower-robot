#include <AFMotor.h>

#define irLeft  A3
#define irRight A0
#define trigPin A2
#define echoPin A1

AF_DCMotor motor1(1, MOTOR12_1KHZ);
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);

int straightSpeed = 100;
int curveFast     = 160;
int lastDirection = 0;
#define LINE_COLOR 1

// ─── الحساسات فالخط؟ ───────────────────────────────
bool onLine() {
  return (digitalRead(irLeft) == LINE_COLOR || digitalRead(irRight) == LINE_COLOR);
}

bool bothOnLine() {
  return (digitalRead(irLeft) == LINE_COLOR && digitalRead(irRight) == LINE_COLOR);
}

void setup() {
  pinMode(irLeft,  INPUT);
  pinMode(irRight, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  long distance = getDistance();

  if (distance > 0 && distance < 20) {
    avoidObstacle();
  } else {
    followLine();
  }
}

// ─── تتبع الخط ──────────────────────────────────────
void followLine() {
  int L = digitalRead(irLeft);
  int R = digitalRead(irRight);

  if (L == LINE_COLOR && R == LINE_COLOR) {
    moveForward(straightSpeed);
    lastDirection = 0;
  } else if (L != LINE_COLOR && R == LINE_COLOR) {
    lastDirection = 1;
    smoothRight();
  } else if (L == LINE_COLOR && R != LINE_COLOR) {
    lastDirection = -1;
    smoothLeft();
  } else {
    recoverLine();
  }
}

// ─── تجاوز العائق ───────────────────────────────────
void avoidObstacle() {
  Serial.println("=== Obstacle detected! ===");

  // 1. وقف
  stopMotors();
  delay(300);

  // 2. رجع لورا شوية
  setMotors(110, 110, BACKWARD, BACKWARD);
  delay(200);
  stopMotors();
  delay(200);

  // 3. لف يسار لحد ما IR اليمين يسيب الخط تماماً
  Serial.println("Step 3: Turn left off line...");
  setMotors(150, 150, BACKWARD, FORWARD);
  unsigned long t = millis();
  while (millis() - t < 600) {          // max 600ms
    if (!onLine()) break;               // لو سبنا الخط نوقف
    delay(10);
  }
  // كمّل اللفة شوية تضمن إننا بعيدين
  delay(250);
  stopMotors();
  delay(200);

  // 4. امشي للأمام تعدي العائق - وابتعد عنه جانبياً
  Serial.println("Step 4: Move forward past obstacle...");
  setMotors(110, 110, FORWARD, FORWARD);
  delay(600);
  stopMotors();
  delay(200);

  // 5. لف يمين ترجع ناحية الخط
  Serial.println("Step 5: Turn right toward line...");
  setMotors(150, 150, FORWARD, BACKWARD);
  t = millis();
  while (millis() - t < 400) {
    delay(10);
  }
  stopMotors();
  delay(200);

  // 6. امشي للأمام وابحث عن الخط
  Serial.println("Step 6: Search for line...");
  t = millis();
  while (!onLine()) {
    setMotors(90, 90, FORWARD, FORWARD);

    // لو فضل 2 ثانية ومش لاقي الخط → sweep يمين/شمال
    if (millis() - t > 2000) {
      Serial.println("Sweeping to find line...");
      sweepForLine();
      break;
    }
    delay(10);
  }

  stopMotors();
  delay(200);
  Serial.println("=== Back on line! ===");
}

// ─── Sweep يمين/شمال لو ضاع الخط ───────────────────
void sweepForLine() {
  // sweep يمين أول
  Serial.println("Sweep RIGHT...");
  setMotors(130, 130, FORWARD, BACKWARD);
  unsigned long t = millis();
  while (!onLine() && millis() - t < 500) {
    delay(10);
  }

  if (onLine()) { stopMotors(); return; }

  // sweep يسار (ضعف الوقت عشان نرجع المركز + شمال)
  Serial.println("Sweep LEFT...");
  setMotors(130, 130, BACKWARD, FORWARD);
  t = millis();
  while (!onLine() && millis() - t < 1000) {
    delay(10);
  }

  if (onLine()) { stopMotors(); return; }

  // آخر حل: ارجع للمركز
  Serial.println("Re-center...");
  setMotors(130, 130, FORWARD, BACKWARD);
  delay(500);
  stopMotors();
}

// ─── دوال الحركة ────────────────────────────────────
void smoothRight() {
  setMotors(curveFast, 0, FORWARD, FORWARD);
  delay(15);
  stopMotors();
  delay(5);
}

void smoothLeft() {
  setMotors(0, curveFast, FORWARD, FORWARD);
  delay(15);
  stopMotors();
  delay(5);
}

void moveForward(int spd) {
  setMotors(spd, spd, FORWARD, FORWARD);
}

void stopMotors() {
  setMotors(0, 0, RELEASE, RELEASE);
}

void recoverLine() {
  if      (lastDirection ==  1) setMotors(140, 0,   FORWARD, FORWARD);
  else if (lastDirection == -1) setMotors(0,   140, FORWARD, FORWARD);
  else                          moveForward(70);
}

long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return duration * 0.034 / 2;
}

void setMotors(int leftSpeed, int rightSpeed, int leftDir, int rightDir) {
  leftSpeed  = constrain(leftSpeed,  0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);

  motor1.setSpeed(leftSpeed);
  motor2.setSpeed(leftSpeed);
  motor1.run(leftDir);
  motor2.run(leftDir);

  motor3.setSpeed(rightSpeed);
  motor4.setSpeed(rightSpeed);
  motor3.run(rightDir);
  motor4.run(rightDir);
}