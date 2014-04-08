//pin 2 on the hbridge is connected to pin 12 on arduino
//pin 7 on hbridge connected to pin 11 on arduino
// pin 11 and 12 on the arduino control motor 1
// pin 15 on the hbridge is connected to pin 8 on arduino
// pin 10 on the hbridge is connected to pin 7 on arduino
// pin 7 and 8 on the arduino control motor 2

// Motor Pins
#define M1_HIGH_PIN 12
#define M1_LOW_PIN 11
#define M2_HIGH_PIN 7
#define M2_LOW_PIN 8

// Comm Pins
#define CARRIER_PIN 5 // carrier only comes out on 5! do not change!
#define DATA_SWITCH_PIN = 22;
#define DATA_FRQ_MICROS = 104.15; // delayMicroseconds() this amount for data frequency

// Collision Pins/State
#define BUMPER_DEBOUNCE_MILLIS 500
int lastBumperTriggerMillis = 0;
int bumperPressPending = false;
typedef enum {NONE, FRONT, RIGHT, LEFT, BACK} bumper_t;
bumper_t pendingBumper = NONE;

// Color Detection
#define RED_LED_PIN 2
#define BLUE_LED_PIN 3
#define COLOR_SENSOR_PIN A1


void setup(){
   
   // Motor Pins
   pinMode(M1_HIGH_PIN, OUTPUT);
   pinMode(M1_LOW_PIN, OUTPUT); 
   pinMode(M2_HIGH_PIN, OUTPUT);
   pinMode(M2_LOW_PIN, OUTPUT);
   
   // Setup Clock for Carrier Frequency
   // see IR sketch for register explanations
   pinMode(5, OUTPUT);
   TCCR3A = _BV(COM3A0) | _BV(COM3B0) | _BV(WGM30) | _BV(WGM31); 
   TCCR3B = _BV(WGM32) | _BV(WGM33) | _BV(CS31);
   OCR3A = 39; // sets the value at which the register resets. 39 generates 25kHz
   digitalWrite(5, HIGH); // turn on the carrier
   
   // Collision
   attachInterrupt(2, frontBumperISR, RISING); // pin 21
   attachInterrupt(3, leftBumperISR, RISING);  // pin 20
   attachInterrupt(4, rightBumperISR, RISING); // pin 19
   attachInterrupt(5, backBumperISR, RISING);  // pin 18
   lastBumperTriggerMillis = millis();
   
   // Color Pins
   pinMode(RED_LED_PIN, OUTPUT);
   pinMode(BLUE_LED_PIN, OUTPUT);
   // TODO: DO we need to set the color sensor pin to input?
 
}

void loop(){
  forward(100);  
  delay(1000);
  stop();
  delay(500);
  reverse(100);
  delay(1000);
  stop();
  delay(500);
  
  turn('l', 90);
  turn('r', 90);
  turn('l',180);
  
  
}

/*
* COLOR DETECTION
*/

int detectColor(int red, int blue, int threshold) {
  int temp;
  if ((red - blue) > threshold) {
    Serial.println("red");
    return 1;
  }
  else if ((blue - red) > threshold){
     Serial.println("blue");
     return 2;
  }
  Serial.println("black");
  return 0;
}

int getRedLedValue() {
  analogWrite(RED_LED_PIN, 255);
  analogWrite(BLUE_LED_PIN, 0);
  delay(100);
  return analogRead(COLOR_SENSOR_PIN;
}

int getBlueLedValue() {
  analogWrite(RED_LED_PIN, 0);
  analogWrite(BLUE_LED_PIN,255);
  delay(100);
  return analogRead(COLOR_SENSOR_PIN);
}

/*
* COLLISION DETECTION
*/

void frontBumperISR() {
  int now = millis();
  if ((now - lastBumperTriggerMillis) < BUMPER_DEBOUNCE_MILLIS) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
  pendingBumper = FRONT;
}

void rightBumperISR() {
  int now = millis();
  if ((now - lastBumperTriggerMillis) < BUMPER_DEBOUNCE_MILLIS) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
  pendingBumper = RIGHT;
}

void leftBumperISR() {
  int now = millis();
  if ((now - lastBumperTriggerMillis) < BUMPER_DEBOUNCE_MILLIS) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
  pendingBumper = LEFT;
}

void backBumperISR() {
  int now = millis();
  if ((now - lastBumperTriggerMillis) < BUMPER_DEBOUNCE_MILLIS) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
  pendingBumper = BACK;
}

/*
* MOTOR CONTROL
*/

void forward(int speed) {
  forward(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void forward(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setHighPin(m1_high, m1_low, speed);
  setLowPin(m2_high, m2_low, speed);
}

void reverse(int speed) {
  reverse(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void reverse(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, speed);
  setHighPin(m2_high, m2_low, speed);
}

void left(int speed) {
  left(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void left(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, speed);
  setLowPin(m2_high, m2_low, speed);
}
void right(int speed) {
  right(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void right(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setHighPin(m1_high, m1_low, speed);
  setHighPin(m2_high, m2_low, speed);
}

void turn(char dir, int angle){
  if(dir == 'l'){
    left(87);
    delay(1200*(angle/90));
    stop();
    delay(200);
  }
  else if(dir == 'r'){
    right(87);
    delay(1200*(angle/90));
    stop();
    delay(200);
  }
}

void stop() {
  stop(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN);
}

void stop(int m1_high, int m1_low, int m2_high, int m2_low) {
  digitalWrite(m1_high, LOW);
  digitalWrite(m2_high, LOW);
  digitalWrite(m1_low, LOW);
  digitalWrite(m2_low, LOW);
}
  

void setHighPin(int high_pin, int low_pin, int speed) {
  if (speed > 255) return;
  
  digitalWrite(low_pin, LOW);
  analogWrite(high_pin, speed);
}

void setLowPin(int high_pin, int low_pin, int speed) {
  if (speed > 255) return;
  
  digitalWrite(high_pin, LOW);
  analogWrite(low_pin, speed);
}
