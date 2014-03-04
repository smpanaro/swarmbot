int red = 11;
int blue = 12;
int switch1 = 2; // blue switch
int switch2 = 3; //red switch
volatile int stateRed;
volatile int stateBlue;
volatile int timeDelay = 1000;
volatile int brightness;
volatile int duty;
boolean pressedRed = false;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
   stateRed = digitalRead(switch1);
   stateBlue = digitalRead(switch2);
  pinMode(red, OUTPUT);
  attachInterrupt(0, toggleRed, CHANGE);
  pinMode(blue, OUTPUT);
  attachInterrupt(1, toggleBlue, CHANGE);
  digitalWrite(red, stateRed);
  digitalWrite(blue, digitalRead(switch1));
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  Serial.println(analogRead(0));
  brightness = analogRead(0);
  duty = brightness/4;
analogWrite(blue, duty);
if(pressedRed ==true)
{
     analogWrite(red, duty);
}
delay(timeDelay);
analogWrite(blue, 0);
delay(timeDelay);
}

void toggleRed() {
  if(timeDelay != 1000)
  {
    stateRed = digitalRead(switch1); 
    if(stateRed == 0)
    {
      pressedRed = true;
      analogWrite(red, duty);
    }
    else
    {
      pressedRed = false; 
      analogWrite(red, 0); 
    }
  }
}

void toggleBlue() {
  if(timeDelay == 100)
  {
    pressedRed = false; 
    timeDelay = 1000;
     stateRed = 0;
     digitalWrite(red, stateRed);
  }
  else
  {
     timeDelay = 100; 
  }
}
