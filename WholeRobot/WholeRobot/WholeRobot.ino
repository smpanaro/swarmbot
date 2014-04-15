#include <TimerOne.h>
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
// all need to be on analogWrite-able pins. (2 - 13 and 44 - 46)
#define M1_HIGH_PIN 6
#define M1_LOW_PIN 4
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
// THE ONLY Constants you should need to tweak are the base values.
// (Turn on color printing to see the typical red and blue values to set the bases to.)
// For whatever reason the first few reads are ~100 higher than all other reads.
#define RED_LED_PIN 31
#define BLUE_LED_PIN 45 // needs to be on one of the analogWrite-able pins.
#define COLOR_SENSOR_PIN A0
#define COLOR_BUFFER_SIZE 10 // higher numbers dampen noise, lower numbers allow for quicker switching time
volatile int numReds = 0;
volatile int numBlacks = COLOR_BUFFER_SIZE;
volatile int numBlues = 0;
volatile int colorBufferIndex = 0;
volatile color_t colorBuffer[COLOR_BUFFER_SIZE]; // dynamically filled in colorCalibration function;
int BLUE_PWM = 0; // Brightness of the blue LED
volatile int RED_BASE = 450;
volatile int BLUE_BASE = 450;

// State 
volatile color_t currentColor = BLACK; // volatile since this is updated in an ISR
state_t currentState = START_STATE;
state_t lastState = (state_t)NULL;

// Configuration
const int FORWARD_SPEED = 70;
const int SEARCH_FORWARD_SPEED = 60;
const int TURN_SPEED = 70;
const int MILLIS_TO_TURN_90 = 500;

const int COLOR_DETECTION_DELTA = 70;

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
   delay(100); // let the sensor rest before the first interrupt (prevents false positive on first reading)
   Timer1.initialize(); // THIS OBLITERATES PINS 9-13 - probe them if you think you can use them.
   Timer1.attachInterrupt(colorSensorISR, 50000); // NOTE: Docs say this breaks analogWrite on digital pins 9 and 10!
   //COLOR_PRINT("blue base:");COLOR_PRINT(BLUE_BASE);
   //COLOR_PRINT(" red base:");COLOR_PRINTLN(RED_BASE);
}

void loop(){
  Serial.print("current state: ");Serial.print(currentState);
  Serial.print("current color: ");Serial.println(currentColor);

  updateState();
  
  switch(currentState){
    case START_STATE: handleStartState(); break;
    case LINE_FOLLOW_STATE: handleLineFollowState(); break;
    case LINE_SEARCH_STATE: handleLineSearchState(); break;
    default: break;
  }
  
  //delay(100); // need this or our loop gets frozen sometimes
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
  if (lastState == START_STATE) {
    //TODO: We might overshoot here. Might need to shuffle ourselves to be on the line.
    stop(); delay(100);
    turn('r', 70);
  }
  
  if (lastState != LINE_FOLLOW_STATE) {
    // Clear last state. No need to go forward and delay if we're already moving.
    lastState = LINE_FOLLOW_STATE;
    forward(SEARCH_FORWARD_SPEED);
    delay(100);  
  }
}

void handleLineSearchState() {
  stop();
  delay(100);
  
  int angle = 90;
  
  // Step 1 - Turn left angle degrees, stopping if we see color.
  left(TURN_SPEED);
  delayWhileColorNotDetected(MILLIS_TO_TURN_90*(float(angle)/90.0));
  stop(); delay(100);
  
  if (currentColor != BLACK) {
    // Back on track, so course correct a tiny bit -- the path isn't perfectly straight.
    turn('l', 5);
    return;
  }
  
  // Step 2 - If no color so far, turn right 2*angle degrees. Again stop if we see color.
  right(TURN_SPEED);
  delayWhileColorNotDetected(MILLIS_TO_TURN_90*(float(2*angle)/90.0));
  stop(); delay(100);
  
  if (currentColor != BLACK) {
    // Back on track, so course correct a tiny bit -- the path isn't perfectly straight.
    turn('r', 5);
    return;
  }
  
  // If we still can't find it, back up a bit (towards the line) and we'll try again on the next loop.
  reverse(SEARCH_FORWARD_SPEED); delay(200);
  stop(); delay(100);
}

// Delay for delayMillis or until a color is detected.
void delayWhileColorNotDetected(unsigned long delayMillis) {
  unsigned long endTime = millis() + delayMillis;
  while (millis() < endTime) {
    if (currentColor != BLACK) break;
  } 
}

/*
* COLOR DETECTION
*/

// Accomplishes two things: pick a value for the BLUE_PWM so that the red and blue values
// are roughly equal on black. Also, fills the color sense buffer with BLACK to initialize it.
void calibrateColorSensing() {
   BLUE_PWM = 0;

   int blueVal = getBlueLedValue(false);
   int redVal = getRedLedValue(false);
   int difference = abs(redVal) - abs(blueVal);
   
   COLOR_PRINT("initial diff:"); COLOR_PRINTLN(difference);

   while (difference > COLOR_DETECTION_DELTA) {
     if (difference > 2 * COLOR_DETECTION_DELTA) BLUE_PWM += 4;
     BLUE_PWM++;
     redVal = getRedLedValue(false);
     blueVal = getBlueLedValue(false);
     difference = abs(redVal) - abs(blueVal);
     COLOR_PRINT("blue pwm:");COLOR_PRINTLN(BLUE_PWM);
     COLOR_PRINT("diff:");COLOR_PRINTLN(difference);
     COLOR_PRINTLN();
     delayMicroseconds(50000); //delay(100);
   }
   
   COLOR_PRINT("FINAL blue pwm:");COLOR_PRINTLN(BLUE_PWM);
   COLOR_PRINT("FINAL diff:");COLOR_PRINTLN(difference);
   
   // Fill the moving average buffer with all BLACK - since that's what we start on.
   for (int i = 0; i < COLOR_BUFFER_SIZE; i++) {
     colorBuffer[i] = BLACK;
   }
   numBlacks = COLOR_BUFFER_SIZE;
   colorBufferIndex = 0;
}

void colorSensorISR() {
  currentColor = detectColor();
}


// Update the color buffer. Pick the color that has appeared the most in the buffe
color_t detectColor() {
  int red = getRedLedValue(true);
  int blue = getBlueLedValue(true);
  digitalWrite(RED_LED_PIN, LOW);
  analogWrite(BLUE_LED_PIN, 0); // disable both leds so that the color sensor is not affected in between reads.
  COLOR_PRINT("red:");COLOR_PRINT(red);
  COLOR_PRINT("blue:");COLOR_PRINTLN(blue);
  
  color_t color = BLACK;
  
  if ((red - COLOR_DETECTION_DELTA) > RED_BASE) {
    color = RED;
  }
  else if ((blue - COLOR_DETECTION_DELTA) > BLUE_BASE){
     color = BLUE;
  }
  
  // Update the buffer with the color we have detected.
  color_t oldColor = colorBuffer[colorBufferIndex];
  colorBuffer[colorBufferIndex] = color;
  colorBufferIndex = (colorBufferIndex + 1) % COLOR_BUFFER_SIZE;
  
  // Update totals
  switch(oldColor) {
    case RED: numReds--; break;
    case BLUE: numBlues--; break;
    case BLACK: numBlacks--; break;
    default: break;
  }
  
  switch(color) {
    case RED: numReds++; break;
    case BLUE: numBlues++; break;
    case BLACK: numBlacks++; break;
    default: break;
  }
  
  // Too many consecutive print statements don't let the interrupt finish in enough time.
  // Don't uncomment all of them or the program will lock up.
    //COLOR_PRINT("numReds:");COLOR_PRINTLN(numReds);
  //COLOR_PRINT("numBlues:");COLOR_PRINTLN(numBlues);
  //COLOR_PRINT("numBlacks:");COLOR_PRINTLN(numBlacks);
  //COLOR_PRINT("currentIdx:");COLOR_PRINTLN(colorBufferIndex);
  //COLOR_PRINT("total:");COLOR_PRINTLN(numReds+numBlues+numBlacks);
  
  if (numReds > numBlues && numReds > numBlacks) return RED;
  else if (numBlues > numReds && numBlues > numBlacks) return BLUE;
  return BLACK;
}

int getRedLedValue(boolean inISR) {
  digitalWrite(RED_LED_PIN, HIGH);
  analogWrite(BLUE_LED_PIN, 0);
  if (!inISR) delayMicroseconds(10000);//delay(100);
  else delayMicroseconds(10000);
  return analogRead(COLOR_SENSOR_PIN);
}

int getBlueLedValue(boolean inISR) {
  digitalWrite(RED_LED_PIN, LOW);
  analogWrite(BLUE_LED_PIN, BLUE_PWM);
  if (!inISR) delayMicroseconds(10000);//delay(100);
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
  setHighPin(m1_high, m1_low, speed);
  setHighPin(m2_high, m2_low, speed);
}
void right(int speed) {
  right(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void right(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, speed);
  setLowPin(m2_high, m2_low, speed);
}

void turn(char dir, int angle){
  if(dir == 'l'){
    left(TURN_SPEED);
    delay(MILLIS_TO_TURN_90*(float(angle)/90.0));
    stop();
    delay(200);
  }
  else if(dir == 'r'){
    right(TURN_SPEED);
    delay(MILLIS_TO_TURN_90*(float(angle)/90.0));
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
