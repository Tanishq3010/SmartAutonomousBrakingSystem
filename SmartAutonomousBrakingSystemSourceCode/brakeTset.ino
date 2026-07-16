#include <AFMotor.h>
#include <Servo.h>

// Ultrasonic sensor pins
const int trigPin = A1;
const int echoPin = A0;

// LED and buzzer pins
const int greenLedPin = A2;
const int redLedPin = A3;
const int buzzerPin = A4;

// Critical stop distance in cm (unchanged)
const int criticalDistance = 25;

// Avoidance maneuver timing
const int reverseTime = 600;
const int turnTime = 400;

// Direction constants
const int LEFT = 0;
const int RIGHT = 1;

// Servo setup
Servo scanServo;
const int servoPin = 10;
const int centerAngle = 90;
const int sweepAngle = 60;
const int servoSettleTime = 400;

// Motor objects
AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

long duration;
float distanceCm;
bool obstacleThisCycle = false;
bool carStopped = false;

bool obstacleCenter = false;
bool obstacleLeft = false;
bool obstacleRight = false;

// NEW: store actual measured left/right distances for comparison
float leftDistance = 0;
float rightDistance = 0;

// ==== Bluetooth control (HC-05 wired to Uno's RX0/TX1, using hardware Serial) ====
char value;

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  scanServo.attach(servoPin);
  scanServo.write(centerAngle);
  delay(500);

  motor1.setSpeed(150);
  motor2.setSpeed(150);
  motor3.setSpeed(150);
  motor4.setSpeed(150);

  moveForward();
  carStopped = false;
}

void loop() {
  // NEW: check for Bluetooth commands first
  Bluetoothcontrol();

  obstacleThisCycle = false;
  obstacleCenter = false;
  obstacleLeft = false;
  obstacleRight = false;

  checkPosition(centerAngle, "Center");
  checkPosition(centerAngle - sweepAngle, "Left");
  checkPosition(centerAngle, "Center");
  checkPosition(centerAngle + sweepAngle, "Right");
  checkPosition(centerAngle, "Center");

  if (obstacleThisCycle) {
    avoidObstacle();
  } else if (carStopped) {
    moveForward();
    carStopped = false;
    Serial.println(">>> Path fully clear. Resuming forward.");
  }
  Serial.println("----------------------------");
}

// NEW: Bluetooth command handler (HC-05 shares the Uno's hardware Serial/USB port)
void Bluetoothcontrol() {
  if (Serial.available() > 0) {
    value = Serial.read();
    Serial.println(value);
  }
  if (value == 'F') {
    moveForward();
    carStopped = false;
  } else if (value == 'B') {
    reverseCar(reverseTime);
  } else if (value == 'L') {
    turnLeft(turnTime);
  } else if (value == 'R') {
    turnRight(turnTime);
  } else if (value == 'S') {
    stopMotors();
    carStopped = true;
  }
}

void checkPosition(int angle, const char* label) {
  scanServo.write(angle);
  delay(servoSettleTime);

  distanceCm = getDistance();

  Serial.print(label);
  Serial.print(" (");
  Serial.print(angle);
  Serial.print(" deg) - Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  // NEW: store left/right readings regardless of obstacle status
  if (String(label) == "Left") leftDistance = distanceCm;
  else if (String(label) == "Right") rightDistance = distanceCm;

  if (distanceCm > 0 && distanceCm < criticalDistance) {
    obstacleThisCycle = true;

    if (String(label) == "Center") obstacleCenter = true;
    else if (String(label) == "Left") obstacleLeft = true;
    else if (String(label) == "Right") obstacleRight = true;

    if (!carStopped) {
      stopMotors();
      carStopped = true;
      Serial.println(">>> Obstacle detected! Stopping immediately.");
    }
  }
}

void avoidObstacle() {
  reverseCar(reverseTime);

  // NEW: choose direction by comparing measured left vs right distance
  int firstChoice, secondChoice;
  Serial.print("Left distance: ");
  Serial.print(leftDistance);
  Serial.print(" cm | Right distance: ");
  Serial.print(rightDistance);
  Serial.println(" cm");

  if (leftDistance > rightDistance) {
    Serial.println(">>> Left side more open. Choosing left first.");
    firstChoice = LEFT;
    secondChoice = RIGHT;
  } else {
    Serial.println(">>> Right side more open. Choosing right first.");
    firstChoice = RIGHT;
    secondChoice = LEFT;
  }

  if (tryDirection(firstChoice)) return;
  if (tryDirection(secondChoice)) return;

  // Both directions blocked - stay stopped, next cycle will re-scan
  stopMotors();
  carStopped = true;
  Serial.println(">>> Both sides blocked. Staying stopped.");
}

bool tryDirection(int dir) {
  if (dir == LEFT) {
    Serial.println(">>> Trying left turn...");
    turnLeft(turnTime);
  } else {
    Serial.println(">>> Trying right turn...");
    turnRight(turnTime);
  }

  scanServo.write(centerAngle);
  delay(servoSettleTime);
  float d = getDistance();

  Serial.print("Recheck after turn - Distance: ");
  Serial.print(d);
  Serial.println(" cm");

  if (d <= 0 || d >= criticalDistance) {
    Serial.println(">>> Direction clear. Continuing forward.");
    moveForward();
    carStopped = false;
    return true;
  } else {
    Serial.println(">>> Still blocked. Undoing turn.");
    if (dir == LEFT) turnRight(turnTime); else turnLeft(turnTime);
    return false;
  }
}

void reverseCar(int ms) {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  delay(ms);
}

void turnRight(int ms) {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  delay(ms);
}

void turnLeft(int ms) {
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
  delay(ms);
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  float dist = (duration * 0.0343) / 2;
  return dist;
}

void moveForward() {
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);

  digitalWrite(greenLedPin, HIGH);
  digitalWrite(redLedPin, LOW);
  digitalWrite(buzzerPin, LOW);
}

void stopMotors() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);
  digitalWrite(buzzerPin, HIGH);
}