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
   if((numOnes<100)&&(numOnes>25)) return GOOD;
   else if((numOnes<200)&&(numOnes>125)) return RETRANSMIT; //this case is when the master receives a message from
                                                     //the slave saying "I received a bad message"
   else if((numOnes<300)&&(numOnes>225)) return RED_FOUND;
   else if((numOnes<400)&&(numOnes>325)) return BLUE_FOUND;
   else if((numOnes<500)&&(numOnes>425)) return DONE;
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
     numOnes = 50;
     break;
     
   case RETRANSMIT:
     numOnes = 150;
     break;
   
   case RED_FOUND:
     numOnes = 250;
     break;
   
   case BLUE_FOUND:
     numOnes = 350;
     break;
    
   case DONE: 
     numOnes = 450;
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
