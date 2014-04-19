///////////SLAVE MODE////////////////
//////////////////////////////////////

int state = 0;
int inPin = 30;
int outPin = 31;
unsigned int dataDelayMicros = 1667; //1/dataFreq (converted to micro seconds)

int RED_COLOR = 0;
int BLUE_COLOR = 1;
int currentColor = BLUE_COLOR;

const int RETRANSMIT = 12345;
const int BAD = 666; // garbled message - not an actual message that can be sent
const int GOOD = 420;
const int RED_FOUND = 555;
const int BLUE_FOUND = 333;
const int DONE = 111;

// Configure the relationship between message and number of ones.
const int MESSAGE_DIFFERENCE = 100; // The number of 1s between two consecutive message definitions.
const int MESSAGE_DETECTION_DELTA = 25; // The value to accept +/- the true value of the message. Should be less than 0.5*MESSAGE_DIFFERENCE
const int goodNumOnes = MESSAGE_DIFFERENCE*(0.5);
const int retransmitNumOnes = MESSAGE_DIFFERENCE*(1.5);
const int redFoundNumOnes = MESSAGE_DIFFERENCE*(2.5);
const int blueFoundNumOnes = MESSAGE_DIFFERENCE*(3.5);
const int doneNumOnes = MESSAGE_DIFFERENCE*(4.5);

int messageTime = 5000; //milliseconds

void setup()
{
  Serial.begin(9600);
  
  pinMode(inPin, INPUT);
  pinMode(outPin, OUTPUT);
}

void loop()
{
    digitalWrite(13, LOW);
  delay(2000);colorFound(BLUE_COLOR);
   //beSlave();
   
   delay(1000 * 60 * 90);
   Serial.println("DONE DONE DONE \n\n\n\n");
  
}

void beSlave()
{
  Serial.println("i am the slave");
  
   while(digitalRead(inPin) == LOW){} //delays until other robot pings us
   Serial.println("ping received");
   pingBack();
   delay(500);
   Serial.println("ready to receive");
   int message = receiveMessage();
   
   while(message == BAD)
   {
    Serial.println("sending RETRANSMIT");
    sendMessage(RETRANSMIT);
    Serial.println("ready to receive again");
    message = receiveMessage(); 
   }   
   Serial.println("sending GOOOD");
   sendMessage(GOOD);   
   lightLED(); 
}

void pingBack()
{
  Serial.println("pinging back");
 while(digitalRead(inPin) == HIGH)
 {
  digitalWrite(outPin, HIGH); 
 }
}

void colorFound(int n)
{
    Serial.println("i am the master");
    pingHandler();
    delay(500);
    int msg;
    if (n == BLUE_COLOR) msg = BLUE_FOUND;
    else msg = RED_FOUND;
    sendMessage(msg);

    while(receiveMessage() == BAD)
    {
      Serial.println("uh oh BAD message, sending again");
      sendMessage(msg);
    }
    lightLED();
    
  
}

void lightLED()
{
 analogWrite(13, 255); 
}

int receiveMessage()
{
  int numOnes = 0;
  int start = millis();
 
  while((millis()-start)< messageTime)
  {
    if(digitalRead(inPin) == HIGH)
      numOnes++;
    delayMicroseconds(dataDelayMicros);
  }
  Serial.print("I received x ones: ");Serial.println(numOnes);
 return decodeMessage(numOnes);
}

int decodeMessage(int numOnes)
{
  if((numOnes < (goodNumOnes + MESSAGE_DETECTION_DELTA)) &&
     (numOnes > (goodNumOnes - MESSAGE_DETECTION_DELTA))) return GOOD;
  else if((numOnes < (retransmitNumOnes + MESSAGE_DETECTION_DELTA)) &&
          (numOnes > (retransmitNumOnes - MESSAGE_DETECTION_DELTA))) return RETRANSMIT; //this case is when the master receives a message from
                                                                                     //the slave saying "I received a bad message"
  else if((numOnes < (redFoundNumOnes + MESSAGE_DETECTION_DELTA)) &&
          (numOnes > (redFoundNumOnes - MESSAGE_DETECTION_DELTA))) return RED_FOUND;
  else if((numOnes < (blueFoundNumOnes + MESSAGE_DETECTION_DELTA)) &&
          (numOnes > (blueFoundNumOnes - MESSAGE_DETECTION_DELTA))) return BLUE_FOUND;
  else if((numOnes < (doneNumOnes + MESSAGE_DETECTION_DELTA)) &&
          (numOnes > (doneNumOnes - MESSAGE_DETECTION_DELTA))) return DONE;
  return BAD; //this is when the slave receives a message it can not decode, i.e. it is bad. 
}


void pingHandler()
{
  Serial.println("sending pings");
   int numOnes = 0;
  
  while(numOnes<5)
 {
    digitalWrite(outPin, HIGH);
    if(digitalRead(inPin) == HIGH) numOnes++;
    delayMicroseconds(dataDelayMicros);
    
 } 
 Serial.println("ping back received");
 digitalWrite(outPin, LOW);
}

void sendMessage(int n)
{
  Serial.println("preparing to send message");
  int numOnes;
  switch(n)
  {
   case GOOD:
     numOnes = goodNumOnes;
     break;
   case RETRANSMIT:
     numOnes = retransmitNumOnes;
     break;
   case RED_FOUND:
     numOnes = redFoundNumOnes;
     break;
   case BLUE_FOUND:
     numOnes = blueFoundNumOnes;
     break;
   case DONE: 
     numOnes = doneNumOnes;
     break;
  }
  transmit(numOnes);  
}

void transmit(int numOnes)
{
  int start = millis();
  for(int i = 0; i<numOnes; i++)
  {
     digitalWrite(outPin, HIGH);
     delayMicroseconds(dataDelayMicros);
  }
  digitalWrite(outPin, LOW);
  
  while((millis()-start)<messageTime){}
  Serial.println("transmission complete");
  
}
