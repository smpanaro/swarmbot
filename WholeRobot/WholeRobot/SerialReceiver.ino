#include <TimerOne.h>



//Messages
const byte BLUE_FOUND = 100; //d
const byte RED_FOUND = 109; //m
const byte DONE_MSG = 120; //x
const byte RECEIVED = 60;
const byte NO_MESSAGE = 0;

//Messages
const byte MY_BLUE_FOUND = 105;
const byte MY_RED_FOUND = 114;
const byte MY_DONE_MSG = 125;
const byte MY_RECEIVED = 65;
const byte MY_NO_MESSAGE = 5;


#define PACKET_MILLIS 500

// Colors
#define RED_COLOR 1
#define BLUE_COLOR 2

#define RED_PIN 52
#define BLUE_PIN 53

#define maskPin 34

// void setup() {
//   //Setup Oscillation
//   pinMode(KPIN, OUTPUT);
//   TCCR3A = _BV(COM3A0) | _BV(COM3B0) | _BV(WGM30) | _BV(WGM31);
//   TCCR3B = _BV(WGM32) | _BV(WGM33) | _BV(CS31);
//   OCR3A = 39; // sets the value at which the register resets. 39 generates 25kHz
//   //Transmit Data Pin
//   Serial2.begin(300);
//   Serial.begin(9600);
//   digitalWrite(RED_PIN, LOW);
//   digitalWrite(BLUE_PIN, LOW);

// }

// void loop() {
//  beSlave();
// // beMaster();

//   delay(100000);


// }

void beMaster()
{
  int color = RED_COLOR;
    colorFound(color);

  lightLED(color);

}

void colorFound(int color)
{
  for (; ; ) {
    byte msg;
    Serial.println("In colorFound");
    mask();
      switch(color)
      {
        case RED_COLOR:
          Serial2.write(MY_RED_FOUND);
          break;

        case BLUE_COLOR:
          Serial2.write(MY_BLUE_FOUND);
          break;
      }
      Serial2.flush();
      unmask();

      delay(PACKET_MILLIS);
      msg = NO_MESSAGE;
      while((Serial2.available()>0)&&(!isValid(msg = Serial2.read())));
      if (isValid(msg)) {
      Serial.print("Received");
      Serial.print(msg,DEC);
      Serial.println();
      } else {
        Serial.println("No data heard, timeout");
      }
      if(msg==RECEIVED){
        return;
      }
  }

  }

void beSlave()
{
  Serial.println("In slave mode");
  boolean valid = false;
  byte m;
  Serial2.flush();
  while(!valid)
  {
    m = receiveMessage();

    if (isValid(m)) valid = true;
  }
  Serial.print("Received message:");
    Serial.println(m);
  pingBack();
  if(m == BLUE_FOUND)
    lightLED(BLUE_COLOR);
  else
    lightLED(RED_COLOR);

  valid = false;
  while(!valid)
  {
    m = receiveMessage();

    if (isValid(m)) valid = true;
  }
  Serial.print("Received message:");
    Serial.println(m);
}

 void finishedMaster()
 {
  for (; ; ) {
    mask();
    byte msg;
    Serial.println("In finishedMaster");

      Serial2.write(MY_DONE_MSG);

      Serial2.flush();
      unmask();

      delay(PACKET_MILLIS);
      msg = NO_MESSAGE;
      while((Serial2.available()>0)&&(!isValid(msg = Serial2.read())));
      if (msg != NO_MESSAGE) {
      Serial.print("Received msg");
      Serial.print(msg,DEC);
      Serial.println();
      } else {
        Serial.println("No data heard, timeout");
      }
      if(msg==RECEIVED){
        return;
      }
  }
 }

void pingBack() {
  mask();
  Serial.println("pinging back");
  Serial2.write(MY_RECEIVED);
  Serial2.flush();
  unmask();
}

void lightLED(int color)
{
  Serial.println("Lighting LED");
 if(color == RED_COLOR)
   analogWrite(RED_PIN, 255);
 else
   analogWrite(BLUE_PIN, 255);
}

byte receiveMessage() {

 if (Serial2.available() > 0) {
     byte msg = Serial2.read();
     return msg;
 }

 return 0;
}

boolean isValid(byte msg) {
  Serial.println(msg);
  return ((msg == BLUE_FOUND) || (msg == RED_FOUND) || (msg == DONE_MSG) || (msg == RECEIVED));
}

void mask()
{
  digitalWrite(maskPin, LOW);

}

void unmask()
{
  digitalWrite(maskPin, HIGH);
}

