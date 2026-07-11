// Ultrasonic sensor distance test — no motors, no Bluetooth, no Servo
// Just checks if the HC-SR04 is wired correctly and Serial Monitor works cleanly

#define Echo A0
#define Trig A1

void setup() {
  Serial.begin(9600);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
}

void loop() {
  int distance = ultrasonic();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(300);   // slow it down so it's easy to read
}

int ultrasonic() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(4);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  long t = pulseIn(Echo, HIGH, 30000);  // 30ms timeout
  if (t == 0) return -1;                  // no echo received
  long cm = t / 58;
  return (int)cm;
}