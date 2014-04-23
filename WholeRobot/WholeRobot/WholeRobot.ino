#include <TimerOne.h>
#include "constants.h" // import custom types and constants
#include "mario.h"

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
// M1 is the right wheel. M2, the left.
#define M1_HIGH_PIN 6
#define M1_LOW_PIN 7
#define M2_HIGH_PIN 8
#define M2_LOW_PIN 9
const float M1_DEFICIT = 0.9552; // The percent of M2's power that M1 should get.

// Comm Pins
#define CARRIER_PIN 5 // carrier only comes out on 5! do not change!
#define DATA_SWITCH_PIN  43
#define DATA_FRQ_MICROS  104.15 // delayMicroseconds() this amount for data frequency

// Collision Pins/State
#define BUMPER_DEBOUNCE_MILLIS 500
volatile unsigned long lastBumperTriggerMillis = 0;
volatile int bumperPressPending = false;
volatile bumper_t pendingBumper = NONE;

// Color Detection
// THE ONLY Constants you should need to tweak are the base values.
// (Turn on color printing to see the typical red and blue values to set the bases to.)
// For whatever reason the first few reads are ~100 higher than all other reads.
#define RED_LED_PIN 3
#define BLUE_LED_PIN 4 // needs to be on one of the analogWrite-able pins.
#define COLOR_SENSOR_PIN A0
#define COLOR_BUFFER_SIZE 10 // higher numbers dampen noise, lower numbers allow for quicker switching time
volatile int numReds = 0;
volatile int numBlacks = COLOR_BUFFER_SIZE; // Buffer is initialized to all black.
volatile int numBlues = 0;
volatile int colorBufferIndex = 0;
volatile color_t colorBuffer[COLOR_BUFFER_SIZE]; // dynamically filled in colorCalibration function;
int BLUE_PWM = 0; // Brightness of the blue LED
volatile int RED_BASE = 290;
volatile int BLUE_BASE = 290;

#define RED_INDICATOR_LED 52 // For indicating when we've found a color.
#define BLUE_INDICATOR_LED 53

// State
volatile color_t lastColor = BLACK;
volatile color_t currentColor = BLACK;
state_t currentState = START_STATE;
state_t lastState = (state_t)NULL;
boolean onReturnTrip = false;

mode_t mode; // Determines whether we should go solo or one of the swarm modes.

// Line searching
unsigned long currentSearchStateEndMillis = 0;
search_state_t nextSearchState = START;

// Solo v. Swarm (Master v. Slave)
// Connect either SLAVE_CONTROL or MASTER_CONTROL to 5V to enable slave or master.
// Otherwise they need to be tied to GND through either SLAVE_GND/MASTER_GND or Arduino ground.
#define SLAVE_GND 24
#define MASTER_GND 25
#define SLAVE_CONTROL 22
#define MASTER_CONTROL 23


// Configuration
const int FORWARD_SPEED = 70;
const int SEARCH_FORWARD_SPEED = 60;
const int TURN_SPEED = 60;
const int MILLIS_TO_TURN_90 = 500;

const int COLOR_DETECTION_DELTA = 50; //70; // Difference from base value to indicate a color's presence.

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
  attachInterrupt(2, backBumperISR, RISING); // pin 21
  attachInterrupt(3, leftBumperISR, RISING);  // pin 20
  attachInterrupt(4, rightBumperISR, RISING); // pin 19
  attachInterrupt(5, frontBumperISR, RISING);  // pin 18
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


  pinMode(BLUE_INDICATOR_LED, OUTPUT);
  pinMode(RED_INDICATOR_LED, OUTPUT);
  // digitalWrite(RED_INDICATOR_LED, HIGH);
  // digitalWrite(BLUE_INDICATOR_LED, LOW);
  pinMode(soundPin, OUTPUT);

  // Determine Mode (Solo, Master or Slave)
  pinMode(SLAVE_GND, OUTPUT); digitalWrite(SLAVE_GND, LOW);
  pinMode(MASTER_GND, OUTPUT); digitalWrite(MASTER_GND, LOW);
  pinMode(SLAVE_CONTROL, INPUT);
  pinMode(MASTER_CONTROL, INPUT);
  if (digitalRead(SLAVE_CONTROL) == HIGH) mode = SLAVE;
  else if (digitalRead(MASTER_CONTROL) == HIGH) mode = MASTER;
  else mode = SOLO;

  // Flash the indicators to indicate mode.
  // Red - Master, Blue - Slave, Both - Solo
  if (mode == SLAVE) {
    digitalWrite(BLUE_INDICATOR_LED, HIGH);
    delay(1000);
    digitalWrite(BLUE_INDICATOR_LED, LOW);
  }
  else if (mode == MASTER) {
    digitalWrite(RED_INDICATOR_LED, HIGH);
    delay(1000);
    digitalWrite(RED_INDICATOR_LED, LOW);

  }
  else if (mode == SOLO) {
    digitalWrite(BLUE_INDICATOR_LED, HIGH);
    digitalWrite(RED_INDICATOR_LED, HIGH);
    delay(1000);
    digitalWrite(BLUE_INDICATOR_LED, LOW);
    digitalWrite(RED_INDICATOR_LED, LOW);
  }
}

void loop(){
  /*Serial.print("current state: ");Serial.print(currentState);
  Serial.print(" current color: ");Serial.print(currentColor);
  Serial.print(" search state: ");Serial.println(((int)nextSearchState)-1);
*/
  /*calibrating turning
  left(TURN_SPEED);
  delay((MILLIS_TO_TURN_90*(float(90)/90.0)));
  stop();
  delay(100000);
  */
  //return;

  updateState();

  switch(currentState){
    case START_STATE: handleStartState(); break;
    case LINE_FOLLOW_STATE: handleLineFollowState(); break;
    case LINE_SEARCH_STATE: handleLineSearchState(); break;
    case FIRST_BUMP_STATE: handleFirstBumpState(); break;
    case SECOND_BUMP_STATE: handleSecondBumpState(); break;
    case END_OF_LINE_STATE: handleEndOfLineState(); break;
    default: break;
  }

  if (mode == SOLO) lightIndicatorLEDs(); // Maybe do this in the ISR - though maybe not since it's for laughs.
  //delay(100); // need this or our loop gets frozen sometimes
}

/*
* STATE MANAGEMENT
*/

void updateState() {
  if (currentState == START_STATE && bumperPressPending) {
    lastState = currentState;
    currentState = FIRST_BUMP_STATE;
  }
  else if (currentState == FIRST_BUMP_STATE && currentColor != BLACK) {
    lastState = currentState;
    currentState = LINE_FOLLOW_STATE;
  }
  else if (currentState == LINE_FOLLOW_STATE && currentColor == BLACK) {
    lastState = currentState;
    currentState = LINE_SEARCH_STATE;
    nextSearchState = START;
  }
  else if (currentState == LINE_SEARCH_STATE &&  currentColor != BLACK) {
    lastState = currentState;
    currentState = LINE_FOLLOW_STATE;
  }
  else if ((currentState & (LINE_SEARCH_STATE|LINE_FOLLOW_STATE)) && bumperPressPending) {
    lastState = currentState;
    currentState = SECOND_BUMP_STATE;
    onReturnTrip = true;
  }
  else if (currentState == SECOND_BUMP_STATE && currentColor == BLACK) {
    lastState = currentState;
    currentState = LINE_SEARCH_STATE;
  }
  else if (currentState == SECOND_BUMP_STATE && currentColor != BLACK) {
    lastState = currentState;
    currentState = LINE_FOLLOW_STATE;
  }
}

void lightIndicatorLEDs() {
  if (currentColor == RED) {
    digitalWrite(RED_INDICATOR_LED, HIGH);
    digitalWrite(BLUE_INDICATOR_LED, LOW);
  }
  else if (currentColor == BLUE) {
    digitalWrite(RED_INDICATOR_LED, LOW);
    digitalWrite(BLUE_INDICATOR_LED, HIGH);
  }
  else {
    digitalWrite(RED_INDICATOR_LED, LOW);
    digitalWrite(BLUE_INDICATOR_LED, LOW);
  }
}

void handleStartState() {
  if (mode == SLAVE) {
    // First, wait to hear from the master about what color they found.
    // Light that indicator LED.

    // Next, wait to hear from the master that they are done.
    // Turn off the indicator LED.

    // Now go.
  }
  // For Master and Solo, we don't need to wait to start.
  forward(FORWARD_SPEED);
  delay(100);
}

void handleFirstBumpState() {
  if (lastState == START_STATE) {
    // Backup from the wall a little.
    reverse(FORWARD_SPEED);delay(350);
    stop(); delay(100);

    // Spin towards the line. (Since we undershoot a bit, it matters which way we turn.)
    if (lastColor == BLUE) turn('r', 160);
    else turn('l', 160);
    stop(); delay(100);

    // Drive until we see the line.
    forward(FORWARD_SPEED);
    delayUntilColor();

    //Now continue over the line - in the process setting lastColor to the color of the line we just crossed.
    delayUntilBlack();
    stop(); delay(100);

    if (mode == MASTER || mode == SLAVE) {
      // Communicate what color we found.
      // Use lastColor since this is after we've stopped on the black.
      // e.g. communicateColor(lastColor);

      //Turn on the indicator LED for the color we found.
    }

    // Turn so that we're facing the right direction on the line.
    if (lastColor == BLUE) left(TURN_SPEED-5);
    else right(TURN_SPEED-5);
    delayUntilColor();
    stop(); delay(100);

    // We should now be facing the far wall, with our color sensor on the line.

    // Clear everything now that we've handled the bump.
    lastState = FIRST_BUMP_STATE;
    bumperPressPending = false;
    pendingBumper = NONE;
  }
}

void handleSecondBumpState() {
  if (lastState & (LINE_SEARCH_STATE|LINE_FOLLOW_STATE)) {
    // Backup from the wall a little.
    reverse(FORWARD_SPEED);delay(500);
    stop(); delay(100);

    // Spin around onto the line.
    right(TURN_SPEED);
    if (currentColor == BLACK) delayUntilColor(); // We are still facing the wall but slightly off the line. Turn onto it.
    // We are now facing the wall with our color sensor on the line.
    delayUntilBlack();
    delayUntilColor();
    stop(); delay(100);

    lastState = SECOND_BUMP_STATE;
    bumperPressPending = false;
    pendingBumper = NONE;
  }
}

void handleEndOfLineState() {
  stop(); delay(100);

  if (mode == MASTER) {
    // Inform the slave that we've finished our run. Yay.

    // Wait for the slave to tell us what color they found.
    // Turn on that indicator LED.

    // Wait for the slave to tell us that they are done.
    // Flash both LEDs 3 times, with a frequency of 1 flash/second. Then turn both LEDs off.
    for (int i = 0; i < 3; i++) {
      digitalWrite(RED_INDICATOR_LED, HIGH);
      digitalWrite(BLUE_INDICATOR_LED, HIGH);
      delay(1000);
      digitalWrite(RED_INDICATOR_LED, LOW);
      digitalWrite(BLUE_INDICATOR_LED, LOW);
    }
  }
  else if (mode == SLAVE) {
    // Turn off indicator LED for 1 second, then turn it back on.

    // Communicate to master that we are done.
  }

  // Victory song.
  Timer1.detachInterrupt();
  // right(70); // This kind of messes up our final position, and accuracy is part of the requirements.
  marioFlag();
  delay(5000);
  stop(); delay(100);

  // Chill, cause we done.
  while(true);
}

void handleLineFollowState() {
  if (lastState != LINE_FOLLOW_STATE) {
    // Clear last state. No need to go forward and delay if we're already moving.
    lastState = LINE_FOLLOW_STATE;
    forward(SEARCH_FORWARD_SPEED);
    delay(100);
  }
}


void handleLineSearchState() {
  stop(); delay(100);

  unsigned long turnTime = 350;
  unsigned long overshootTime = 150;

  left(60);
  delayWhileColorNotDetected(turnTime);
  stop(); delay(100);

  if (currentColor != BLACK) {
    // fix overshoot
    right(60);
    delay(overshootTime);
    stop(); delay(100);
    return;
  }

  right(60);
  delayWhileColorNotDetected(2*turnTime);
  stop(); delay(100);

  if (currentColor != BLACK) {
    left(60);
    delay(overshootTime);
    stop(); delay(100);
    return;
  }

  left(60);
  delay(turnTime);
  stop(); delay(100);

  if (!onReturnTrip) {
    reverse(60); delay(200);
    stop(); delay(100);
  }

  if (onReturnTrip){
    reverse(60);
    delayWhileColorNotDetected(500); // 500 is max time to back up.
    stop(); delay(100);
    forward(60); delay(overshootTime);
    stop(); delay(100);
    currentState = END_OF_LINE_STATE;
  }

}


void handleLineSearchStateOLDVERSION() {
  if (currentColor != BLACK) {
    // No need to explicitly course correct because our color sensor doesn't immediately update.
    // We've likely rotated the extra amount we need anyways.
    stop(); delay(100);

    if (nextSearchState == PIVOT_TO_ORIG_POS) { // We found color while turning right.
      turn('r', 10);
    }
    else if (nextSearchState == PIVOT_RIGHT) { // We found color while turning left.
      turn('l', 10);
    }
    else if (nextSearchState == START) {} // We found color while backing up.

    return;
  }

  int angle = 30;
  boolean shouldAdvanceStates = (millis() > currentSearchStateEndMillis);

  // WARNING: The inclusion shouldAdvanceState in the below if statement is UNTESTED.
  if (shouldAdvanceStates && nextSearchState == START) {
    stop();
    delay(100);
    nextSearchState = PIVOT_LEFT;
  }
  else if (nextSearchState == PIVOT_LEFT) {
    // Step 1 - Turn left angle degrees, stopping if we see color.
    left(TURN_SPEED);
    nextSearchState = PIVOT_RIGHT;
    currentSearchStateEndMillis = millis() + (MILLIS_TO_TURN_90*(float(angle)/90.0));
  }
  else if (shouldAdvanceStates && nextSearchState == PIVOT_RIGHT ) {
    stop(); delay(100);
    // Step 2 - If no color so far, turn right 2*angle degrees. Again stop if we see color.
    right(TURN_SPEED);
    nextSearchState = PIVOT_TO_ORIG_POS;
    currentSearchStateEndMillis = millis() + (MILLIS_TO_TURN_90*(float(2*angle)/90.0));
  }
  else if (shouldAdvanceStates && nextSearchState == PIVOT_TO_ORIG_POS) {
    stop(); delay(100);
    // Step 3 - Pivot left angle degrees to our original position, so that we can reverse for the next try.
    left(TURN_SPEED);
    nextSearchState = REVERSE;
    currentSearchStateEndMillis = millis() + (MILLIS_TO_TURN_90*(float(angle)/90.0));
  }
  else if (shouldAdvanceStates && nextSearchState == REVERSE) {
    stop(); delay(100);
    // Step 4 - If we still can't find it, back up a bit (towards the line) and we'll try again on the next loop.
    reverse(55); //SEARCH_FORWARD_SPEED);
    nextSearchState = DONE;


    // ALL THE HACKS - srsly, this needs to be removed. and the states below need to start working correctly.
    //currentState = END_OF_LINE_STATE;

    currentSearchStateEndMillis = millis(); // + 500;
    delayUntilColor();
  }
  else if (shouldAdvanceStates && nextSearchState == DONE) {
    // Step 5a - If we are heading toward the wall, drive forward a bit to hit it.
//    if (!onReturnTrip) forward(SEARCH_FORWARD_SPEED); delayUntilBump();

    // Step 5b - If we are on our return trip and we have swept 180 degress without finding color, we're probably done.
    if (onReturnTrip) {
      stop(); delay(100);
      currentState = END_OF_LINE_STATE;
    }


    nextSearchState = START; // shouldn't matter
  }
}

// Delay for delayMillis or until a color is detected.
void delayWhileColorNotDetected(unsigned long delayMillis) {
  unsigned long endTime = millis() + delayMillis;
  while (millis() < endTime) {
    if (currentColor != BLACK) break;
  }
}

void delayUntilBlack() {
  while (currentColor != BLACK){}
}

void delayUntilColor() {
  while (currentColor == BLACK){}
}

void delayUntilBump() {
  while(!bumperPressPending) {};
}

// Delay for delayMillis unless a bumper is pressed. Return whether a bumper was pressed.
boolean bumperPressedWhileDelaying(unsigned long delayMillis) {
  unsigned long endTime = millis() + delayMillis;
  while (millis() < endTime) {
    if (bumperPressPending) return true;
  }
  return false;
}

boolean delayUnlessBumperOrColor(unsigned long delayMillis) {
    unsigned long endTime = millis() + delayMillis;
  while (millis() < endTime) {
    if (bumperPressPending) return true;
    if (currentColor != BLACK) break;
  }
  return false;
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
  color_t newColor = detectColor();
  if (newColor != currentColor) {
    // Toggle indicator LEDs (for debugging and fun).
    if (mode == SOLO) {
      if (lastColor == RED) digitalWrite(RED_INDICATOR_LED, LOW);
      else if (lastColor == BLUE) digitalWrite(BLUE_INDICATOR_LED, LOW);
      if (currentColor == RED) digitalWrite(RED_INDICATOR_LED, HIGH);
      else if (currentColor == BLUE) digitalWrite(BLUE_INDICATOR_LED, HIGH);
    }

    // If the color we just read is different from the last color we were on, update lastColor.
    lastColor = currentColor;
  }
  currentColor = newColor;
}


// Update the color buffer. Pick the color that has appeared the most in the buffer.
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
  unsigned long now = millis();
  if ((now - lastBumperTriggerMillis) < BUMPER_DEBOUNCE_MILLIS) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
  pendingBumper = FRONT;
}

void rightBumperISR() {
  unsigned long now = millis();
  if ((now - lastBumperTriggerMillis) < BUMPER_DEBOUNCE_MILLIS) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
  pendingBumper = RIGHT;
}

void leftBumperISR() {
  unsigned long now = millis();
  if ((now - lastBumperTriggerMillis) < BUMPER_DEBOUNCE_MILLIS) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
  pendingBumper = LEFT;
}

void backBumperISR() {
  unsigned long now = millis();
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
  setHighPin(m1_high, m1_low, speed*M1_DEFICIT);
  setLowPin(m2_high, m2_low, speed);
}

void reverse(int speed) {
  reverse(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void reverse(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, speed*M1_DEFICIT);
  setHighPin(m2_high, m2_low, speed);
}

void left(int speed) {
  left(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void left(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setHighPin(m1_high, m1_low, speed*M1_DEFICIT);
  setHighPin(m2_high, m2_low, speed);
}
void right(int speed) {
  right(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void right(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, speed*M1_DEFICIT);
  setLowPin(m2_high, m2_low, speed);
}

void pivot_left(int speed) {
  pivot_left(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void pivot_left(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setHighPin(m1_high, m1_low, speed*M1_DEFICIT);
  setHighPin(m2_high, m2_low, 0);
}
void pivot_right(int speed) {
  pivot_right(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void pivot_right(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, 0);
  setLowPin(m2_high, m2_low, speed);
}

void rev_pivot_left(int speed) {
  rev_pivot_left(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void rev_pivot_left(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, speed*M1_DEFICIT);
  setHighPin(m2_high, m2_low, 0);
}
void rev_pivot_right(int speed) {
  rev_pivot_right(M1_HIGH_PIN, M1_LOW_PIN, M2_HIGH_PIN, M2_LOW_PIN, speed);
}

void rev_pivot_right(int m1_high, int m1_low, int m2_high, int m2_low, int speed) {
  setLowPin(m1_high, m1_low, 0);
  setHighPin(m2_high, m2_low, speed);
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
