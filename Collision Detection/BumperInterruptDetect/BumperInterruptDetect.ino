// ignore bumper presses for this number of millis after a press is detected - debounces circuit.
int bumperThresholdMillis = 500;
int lastBumperTriggerMillis = 0;
int bumperPressPending = false;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  // Switch input on pin 21
  attachInterrupt(2, bumperISR, RISING);
  lastBumperTriggerMillis = millis();
}

void loop() {
  
  if (bumperPressPending == true) {
    bumperPressPending = false;
    Serial.println("pressed");
  }
  
  delay(10);        
}

void bumperISR() {
  int now = millis();
  if ((now - lastBumperTriggerMillis) < bumperThresholdMillis) return;
  lastBumperTriggerMillis = now;
  bumperPressPending = true;
}


