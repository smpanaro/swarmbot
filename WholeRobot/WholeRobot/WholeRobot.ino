#include <TimerOne.h>
#include "constants.h" // import custom types and constants

// Debug Statements
#define DEBUG_COLOR // uncomment this line to enable printing color sensing debug statements
#ifdef DEBUG_COLOR
  #define COLOR_PRINT(x) Serial.print (x)
  #define COLOR_PRINTLN(x)  Serial.println (x)
#else
  #define COLOR_PRINT(x)
  #define COLOR_PRINTLN(x)
#endif

// Motor Pins
// all need to be on analogWrite-able pins. (2 - 13 and 44 - 46)
#define M1_HIGH_PIN 3
#define M1_LOW_PIN 2
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
#define RED_LED_PIN 31
#define BLUE_LED_PIN 45 // needs to be on one of the analogWrite-able pins.
#define COLOR_SENSOR_PIN A0
int BLUE_PWM = 0; // Brightness of the blue LED ~13 during the day, ~53 at night

// State 
volatile color_t currentColor = BLACK; // volatile since this is updated in an ISR
state_t currentState = START_STATE;
state_t lastState = (state_t)NULL;

// Configuration
const int FORWARD_SPEED = 70;
const int COLOR_DETECTION_DELTA = 100;

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
   delay(100); // let the sensor rest before the first interrupt
   Timer1.initialize(); // THIS OBLITERATES PINS 9-13
   Timer1.attachInterrupt(colorSensorISR, 250000); // NOTE: This breaks analogWrite on digital pins 9 and 10!
}

void loop(){
  Serial.print("current state: ");Serial.println(currentState);
  Serial.print("current color: ");Serial.println(currentColor);

  //currentColor = detectColor(COLOR_DETECTION_DELTA);
  delay(1000); return;

  updateState();
  
  switch(currentState){
    case START_STATE: handleStartState(); break;
    case LINE_FOLLOW_STATE: handleLineFollowState(); break;
    case LINE_SEARCH_STATE: handleLineSearchState(); break;
    default: break;
  }
}

/*
* STATE MANAGEMENT
*/

void updateState() {
  if (currentState == START_STATE && currentColor != BLACK) {
    lastState = currentState;
    currentState = LINE_FOLLOW_STATE;
  } 
  else if (currentState == LINE_FOLLOW_STATE && currentColor == BLACK) {
    lastState = currentState;
    currentState = LINE_SEARCH_STATE;
  }
  else if (currentState == LINE_SEARCH_STATE &&  currentColor != BLACK) {
    lastState = currentState;
    currentState = LINE_FOLLOW_STATE;
  }
}

void handleStartState() {
  forward(FORWARD_SPEED);
  delay(100);
}

void handleLineFollowState() {
  stop();
  delay(100);
}

void handleLineSearchState() {
  stop();
}

/*
* COLOR DETECTION
*/

void calibrateColorSensing() {
   BLUE_PWM = 0;
   int redVal = getRedLedValue(false);
   int blueVal = getBlueLedValue(false);
   int difference = abs(redVal) - abs(blueVal);

   while (difference > COLOR_DETECTION_DELTA) {
     if (difference > 2 * COLOR_DETECTION_DELTA) BLUE_PWM += 4;
     BLUE_PWM++;
     redVal = getRedLedValue(false);
     blueVal = getBlueLedValue(false);
     difference = abs(redVal) - abs(blueVal);
     COLOR_PRINT("blue pwm:");COLOR_PRINTLN(BLUE_PWM);
     COLOR_PRINT("diff:");COLOR_PRINTLN(difference);
     COLOR_PRINTLN();
     delay(100);
   }
   
   // no matter the difference we want red to be higher than blue (see how detectColor works for why)
   if (redVal > blueVal) BLUE_PWM--;
   
   // TODO: Potentially set color detection threshold based on the difference?
   COLOR_PRINT("FINAL blue pwm:");COLOR_PRINTLN(BLUE_PWM);
   COLOR_PRINT("FINAL diff:");COLOR_PRINTLN(difference);
}


void colorSensorISR() {
  currentColor = detectColor(COLOR_DETECTION_DELTA);
}
  
color_t detectColor(int threshold) {
  int red = getRedLedValue(true);
  int blue = getBlueLedValue(true);
  digitalWrite(RED_LED_PIN, LOW);
  analogWrite(BLUE_LED_PIN, 0); // disable both leds so that the color sensor is not affected in between reads.
  COLOR_PRINT("red:");COLOR_PRINTLN(red);
  COLOR_PRINT("blue:");COLOR_PRINTLN(blue);
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

int getRedLedValue(boolean inISR) {
  digitalWrite(RED_LED_PIN, HIGH);
  analogWrite(BLUE_LED_PIN, 0);
  // delay(100) when we're calibrating so we get a very stable read.
 // delayMicroseconds works in the ISR and we don't need a long delay. - 16383 is the max value. 
  if (!inISR) delay(100);
  else delayMicroseconds(10000);
  return analogRead(COLOR_SENSOR_PIN);
}

int getBlueLedValue(boolean inISR) {
  digitalWrite(RED_LED_PIN, LOW);
  analogWrite(BLUE_LED_PIN, BLUE_PWM);
  if (!inISR) delay(100);
  else delayMicroseconds(10000);
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
