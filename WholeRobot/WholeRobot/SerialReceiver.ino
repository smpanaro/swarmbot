#include <TimerOne.h>


#define KPIN 5
#define RXMASKPIN 40
#define DEBUGPIN 7
#define DEBUGLED 13
//1600
#define DATARATE 1600
#define TESTDATARATE 250
#define COMMINTERRUPT 0 //pin 2

//Communication Defines
//error status
#define DATA_OK 0
#define BAD_SYNC_BYTE 8
#define BAD_DATA_BYTE 9
#define TIMEOUT 10
#define BAD_EOM_BYTE 13
//messages
#define ACK 0
#define NACK 1
#define PING 2
#define FOUND_BLUE 3
#define FOUND_RED 4
#define COLLISION 5
#define DONE 6
#define NACK_SYNCBYTE 7
#define NO_DATA_READY 255
//easy way to change active high/low
#define ONE 1
#define ZERO 0
//Byte definitions
#define SYNC_BYTE 0xA6
#define ZERO_DATA_BYTE 0xCC
#define ONE_DATA_BYTE 0xAA
#define EOM_BYTE 0xFF

//txState Values
#define CHILLIN 0
#define SENDING 1
#define WAITFORACK 2
#define NEEDTOACK 3
#define ACKING 4
#define NEEDTONACK 5
#define RECEIVING 6
#define SERIALERROR 7

//masked
#define UNMASKED HIGH
#define MASKED LOW

//Messages
const byte BLUE_FOUND = 100; //d
const byte RED_FOUND = 109; //m
const byte DONE_MSG = 120; //x
const byte RECEIVED = 60;
const byte NO_MESSAGE = 0;


#define PACKET_MILLIS 50

// Colors
#define RED_COLOR 1
#define BLUE_COLOR 2

#define RED_PIN 52
#define BLUE_PIN 53

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
      switch(color)
      {
        case RED_COLOR:
          Serial2.write(RED_FOUND);
          break;

        case BLUE_COLOR:
          Serial2.write(BLUE_FOUND);
          break;
      }
      Serial2.flush();
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

void beSlave()
{
  Serial.println("In slave mode");
  boolean valid = false;
  byte m;

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


  while(!valid)
  {
    m = receiveMessage();

    if (isValid(m)) valid = true;
  }
}

 void finishedMaster()
 {
  for (; ; ) {
    byte msg;
    Serial.println("In finishedMaster");

      Serial2.write(DONE_MSG);

      Serial2.flush();
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

void pingBack()
{

  Serial2.write(RECEIVED);
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
  return ((msg == BLUE_FOUND) || (msg == RED_FOUND) || (msg == DONE_MSG) || (msg == RECEIVED));
}
