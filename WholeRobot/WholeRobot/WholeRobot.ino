#include "constants.h" // import custom types and constants

// Debug Statements
//#define DEBUG_COLOR // uncomment this line to enable printing color sensing debug statements
#ifdef DEBUG_COLOR
  #define COLOR_PRINT(x) Serial.print (x)
  #define COLOR_PRINTLN(x)  Serial.println (x)
#else
  #define COLOR_PRINT(x)
  #define COLOR_PRINTLN(x)
#endif

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
bumper_t pendingBumper = NONE;

// Color Detection
#define RED_LED_PIN 2
#define BLUE_LED_PIN 3
#define COLOR_SENSOR_PIN A0
int BLUE_PWM = 0; // Brightness of the blue LED

// State 
color_t currentColor = BLACK;
state_t currentState = START_STATE;
state_t lastState = (state_t)NULL;


void setup(){
   
  Serial.begin(9600);
  
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
   calibrateColorSensing(); // this is a long ish (2-3 second) function
}

void loop(){
  Serial.print("current state: ");Serial.println(currentState);
  Serial.print("current color: ");Serial.println(currentColor);
  
  currentColor = detectColor(60);
  
  updateState();
  
  switch(currentState){
    case START_STATE: handleStartState(); break;
    case LINE_FOLLOW_STATE: handleLineFollowState(); break;
    default: break;
  }
  
  delay(100);
}

/*
* STATE MANAGEMENT
*/

void updateState() {
  if (currentState == START_STATE && currentColor != BLACK) {
    lastState = currentState;
    currentState = LINE_FOLLOW_STATE;
  }
}

void handleStartState() {
  forward(70);
  delay(100);
}

void handleLineFollowState() {
  stop();
  delay(100);
}

/*
* COLOR DETECTION
*/

void calibrateColorSensing() {
   BLUE_PWM = 0;
   int difference = getRedLedValue() - getBlueLedValue();;
   int last_difference = abs(difference) + 1;
   
   while (abs(last_difference) > abs(difference)) {
     BLUE_PWM++;
     last_difference = difference;
     difference = getRedLedValue() - getBlueLedValue();
       COLOR_PRINT("blue pwm:");COLOR_PRINTLN(BLUE_PWM);
       COLOR_PRINT("diff:");COLOR_PRINTLN(difference);
       COLOR_PRINT("last diff:");COLOR_PRINTLN(last_difference);
       COLOR_PRINTLN();
     delay(100);
   }
   
   BLUE_PWM--;   
   difference = getRedLedValue() - getBlueLedValue();
   
   // TODO: Potentially set color detection threshold based on the difference?
   COLOR_PRINT("FINAL blue pwm:");COLOR_PRINTLN(BLUE_PWM);
   COLOR_PRINT("FINAL diff:");COLOR_PRINTLN(difference);
}
  
  
color_t detectColor(int threshold) {
  int red = getRedLedValue();
  int blue = getBlueLedValue();
  analogWrite(RED_LED_PIN, 0);
  analogWrite(BLUE_LED_PIN, 0); // disable both leds so that the color sensor is not affected in between reads.
  //COLOR_PRINT("red:");COLOR_PRINTLN(red);
  //COLOR_PRINT("blue:");COLOR_PRINTLN(blue);
  if ((red - blue) > threshold) {
    COLOR_PRINTLN("red");
    return RED;
  }
  else if ((blue - red) > threshold){
     COLOR_PRINTLN("blue");
     return BLUE;
  }
  COLOR_PRINTLN("black");
  return BLACK;
}

int getRedLedValue() {
  analogWrite(RED_LED_PIN, 255);
  analogWrite(BLUE_LED_PIN, 0);
  delay(100);
  return analogRead(COLOR_SENSOR_PIN);
}

int getBlueLedValue() {
  analogWrite(RED_LED_PIN, 0);
  analogWrite(BLUE_LED_PIN, BLUE_PWM);
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
  if (speed > 255) speed = 255;
  
  digitalWrite(low_pin, LOW);
  analogWrite(high_pin, speed);
}

void setLowPin(int high_pin, int low_pin, int speed) {
  if (speed > 255) speed = 255;
  
  digitalWrite(high_pin, LOW);
  analogWrite(low_pin, speed);
}
