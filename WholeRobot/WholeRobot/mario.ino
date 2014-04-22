//#define soundPin 8
#include "mario.h"

float C2=	 65.41	;
float Db2 =	 69.30	;
float D2	= 73.42	;
float  Eb2 =	 77.78	;
float E2	= 82.41	 ;
float F2	= 87.31	;
float  Gb2 =	 92.50	;
float G2	= 98.00	;
float  Ab2 =	 103.83	;
float myA2	= 110.00	;
float  Bb2 	= 116.54;
float B2	= 123.47	;
float C3	= 130.81	;
float Db3 =	 138.59	 ;
float D3	= 146.83	 ;
float  Eb3 =	 155.56	 ;
float  E3	= 164.81	 ;
float F3	= 174.61	;
float  Gb3 =	 185.00	;
 float G3 = 196.00;	 
 float Ab3 = 207.65; 
 float myA3 = 220.00;
 float Bb3 = 233.08 ;
 float B3 = 246.94; 
 float C4 =261.63	;
 float Db4 = 277.18	 ;
 float D4= 293.66;	 
 float Eb4  = 311.13;	 
 float E4= 329.63	;
 float F4= 349.23	;
 float Gb4 =369.99	;
 float G4= 392.00	 ;
 float Ab4 = 415.30	 ;
 float myA4= 440.00	 ;
 float Bb4 = 466.16	 ;
 float B4= 493.88;	 
 float C5= 523.25	 ;
 float Db5 = 554.37	 ;
 float D5= 587.33	 ;
 float Eb5 = 622.25	;
 float E5= 659.25	 ;
 float F5= 698.46;	 
 float Gb5 = 739.99	;
 float G5= 783.99;
 float Ab5 	= 830.61	;
float myA5	= 880.00	 ;
float Bb5 =	 932.33	 ;
float B5	= 987.77	 ;
float C6	 = 1046.50;
float Db6 	= 1108.73;//	 31.12
float D6	= 1174.66	;// 29.37
float  Eb6 	= 1244.51;//	 27.72
float E6	= 1318.51	;// 26.17


/*void setup(){
 pinMode(soundPin, OUTPUT);
 
}

void loop()
{
 marioFlag(); 
// otherMario();
 //underWorld();
 delay(100000);
}*/


void underWorld()
{
   tone(soundPin, C3, 500);delay(100);
   tone(soundPin, C4, 500);delay(100);
   tone(soundPin, myA2, 500);delay(100);
   tone(soundPin, myA3, 500);delay(100);
    tone(soundPin, Bb2, 500);delay(100);
   tone(soundPin, Bb3, 500);delay(1000);
 
 tone(soundPin, C3, 500);delay(100);
   tone(soundPin, C4, 500);delay(100);
   tone(soundPin, myA2, 500);delay(100);
   tone(soundPin, myA3, 500);delay(100);
   tone(soundPin, Bb2, 500);delay(100);
   tone(soundPin, Bb3, 500);delay(1000);
   
   tone(soundPin, F2, 500);delay(100);
   tone(soundPin, F3, 500);delay(100);
   tone(soundPin, myA2, 500);delay(100);
   tone(soundPin, myA3, 500);delay(100);
    tone(soundPin, Bb2, 500);delay(100);
   tone(soundPin, Bb3, 500);delay(1000);
 
 tone(soundPin, C3, 500);delay(100);
   tone(soundPin, C4, 500);delay(100);
   tone(soundPin, myA2, 500);delay(100);
   tone(soundPin, myA3, 500);delay(100);
   tone(soundPin, Bb2, 500);delay(100);
   tone(soundPin, Bb3, 500);delay(1000);
  
  
  
  
  
  
  
}
void marioFlag()
{
   tone(soundPin, G3, 500);delay(100);
   tone(soundPin, C4, 500);delay(100);
   tone(soundPin, E4, 500);delay(100);
   tone(soundPin, G4, 500);delay(100);
   tone(soundPin, C5, 500);delay(100);
   tone(soundPin, E5, 500);delay(100);
   tone(soundPin, G5, 500);delay(350);
   tone(soundPin, E5, 500);delay(400);
   
   
   tone(soundPin, Ab3, 500);delay(100);
   tone(soundPin, C4, 500);delay(100);
   tone(soundPin, Eb4, 500);delay(100);
   tone(soundPin, Ab4, 500);delay(100);
   tone(soundPin, C5, 500);delay(100);
   tone(soundPin, Eb5, 500);delay(100);
   tone(soundPin, Ab5, 500);delay(350);
   tone(soundPin, Eb5, 500);delay(400);
  
   tone(soundPin, Bb3, 500);delay(100);
   tone(soundPin, D4, 500);delay(100);
   tone(soundPin, F4, 500);delay(100);
   tone(soundPin, Bb4, 500);delay(100);
   tone(soundPin, D5, 500);delay(100);
   tone(soundPin, F5, 500);delay(100);
   tone(soundPin, Bb5, 380);delay(400);
  
   tone(soundPin, Bb5, 100);delay(125);
   tone(soundPin, Bb5, 100);delay(125);
   tone(soundPin, Bb5, 100);delay(125);
   tone(soundPin, C6, 1000);delay(1000);
   

}

void otherMario()
{
tone(soundPin, 660,200);
delay(75);tone(soundPin, 660,400);
delay(150);tone(soundPin, 660,400);
delay(150);tone(soundPin, 510,400);
delay(50);tone(soundPin, 660,400);
delay(150);tone(soundPin, 770,400);
delay(275);tone(soundPin, 380,400);
delay(287);tone(soundPin, 510,400);
delay(225);tone(soundPin, 380,400);
delay(200);tone(soundPin, 320,400);
delay(250);tone(soundPin, 440,400);
delay(150);tone(soundPin, 480,320);
delay(165);tone(soundPin, 450,400);
delay(75);tone(soundPin, 430,400);
delay(150);tone(soundPin, 380,400);
delay(100);tone(soundPin, 660,320);
delay(100);tone(soundPin, 760,200);
delay(75);tone(soundPin, 860,400);
delay(150);tone(soundPin, 700,320);
delay(75);tone(soundPin, 760,200);
delay(175);tone(soundPin, 660,320);
delay(150);tone(soundPin, 520,320);
delay(75);tone(soundPin, 580,320);
delay(75);tone(soundPin, 480,320);
delay(175);tone(soundPin, 510,400);
delay(275);tone(soundPin, 380,400);
delay(200);tone(soundPin, 320,400);
delay(250);tone(soundPin, 440,400);
delay(150);tone(soundPin, 480,320);
delay(165);tone(soundPin, 450,400);
delay(75);tone(soundPin, 430,400);
delay(150);tone(soundPin, 380,400);
delay(100);tone(soundPin, 660,320);

/////////////////////
delay(100);tone(soundPin, 760,50);
delay(75);tone(soundPin, 860,100);
delay(150);tone(soundPin, 700,80);
delay(75);tone(soundPin, 760,50);
delay(175);tone(soundPin, 660,80);
delay(150);tone(soundPin, 520,80);
delay(75);tone(soundPin, 580,80);
delay(75);tone(soundPin, 480,80);
delay(250);tone(soundPin, 500,100);
delay(150);tone(soundPin, 760,100);
delay(50);tone(soundPin, 720,100);
delay(75);tone(soundPin, 680,100);
delay(75);tone(soundPin, 620,150);
delay(150);tone(soundPin, 650,150);
delay(150);tone(soundPin, 380,100);
delay(75);tone(soundPin, 430,100);
delay(75);tone(soundPin, 500,100);
delay(150);tone(soundPin, 430,100);
delay(75);tone(soundPin, 500,100);
delay(50);tone(soundPin, 570,100);
delay(110);tone(soundPin, 500,100);
delay(150);tone(soundPin, 760,100);
delay(50);tone(soundPin, 720,100);
delay(75);tone(soundPin, 680,100);
delay(75);tone(soundPin, 620,150);
delay(150);tone(soundPin, 650,200);
delay(150);tone(soundPin, 1020,80);
delay(150);tone(soundPin, 1020,80);
delay(75);tone(soundPin, 1020,80);
delay(150);tone(soundPin, 380,100);
delay(150);tone(soundPin, 500,100);
delay(150);tone(soundPin, 760,100);
delay(50);tone(soundPin, 720,100);
delay(75);tone(soundPin, 680,100);
delay(75);tone(soundPin, 620,150);
delay(150);tone(soundPin, 650,150);
delay(150);tone(soundPin, 380,100);
delay(75);tone(soundPin, 430,100);
delay(75);tone(soundPin, 500,100);
delay(150);tone(soundPin, 430,100);
delay(75);tone(soundPin, 500,100);
delay(50);tone(soundPin, 570,100);
delay(110);tone(soundPin, 500,100);
delay(150);tone(soundPin, 760,100);
delay(50);tone(soundPin, 720,100);
delay(75);tone(soundPin, 680,100);
delay(75);tone(soundPin, 620,150);
delay(150);tone(soundPin, 650,200);
delay(150);tone(soundPin, 1020,80);
delay(150);tone(soundPin, 1020,80);
delay(75);tone(soundPin, 1020,80);
delay(150);tone(soundPin, 380,100);
delay(150);tone(soundPin, 500,100);
delay(150);tone(soundPin, 760,100);
delay(50);tone(soundPin, 720,100);
delay(75);tone(soundPin, 680,100);
delay(75);tone(soundPin, 620,150);
delay(150);tone(soundPin, 650,150);
delay(150);tone(soundPin, 380,100);
delay(75);tone(soundPin, 430,100);
delay(75);tone(soundPin, 500,100);
delay(150);tone(soundPin, 430,100);
delay(75);tone(soundPin, 500,100);
delay(50);tone(soundPin, 570,100);
delay(210);tone(soundPin, 585,100);
delay(275);tone(soundPin, 550,100);
delay(210);tone(soundPin, 500,100);
delay(180);tone(soundPin, 380,100);
delay(150);tone(soundPin, 500,100);
delay(150);tone(soundPin, 500,100);
delay(75);tone(soundPin, 500,100);
delay(150);tone(soundPin, 500,60);
delay(75);tone(soundPin, 500,80);
delay(150);tone(soundPin, 500,60);
delay(175);tone(soundPin, 500,80);
delay(75);tone(soundPin, 580,80);
delay(175);tone(soundPin, 660,80);
delay(75);tone(soundPin, 500,80);
delay(150);tone(soundPin, 430,80);
delay(75);tone(soundPin, 380,80);
delay(300);tone(soundPin, 500,60);
delay(75);tone(soundPin, 500,80);
delay(150);tone(soundPin, 500,60);
delay(175);tone(soundPin, 500,80);
delay(75);tone(soundPin, 580,80);
delay(75);tone(soundPin, 660,80);
delay(225);tone(soundPin, 870,80);
delay(162);tone(soundPin, 760,80);
delay(300);tone(soundPin, 500,60);
delay(75);tone(soundPin, 500,80);
delay(150);tone(soundPin, 500,60);
delay(175);tone(soundPin, 500,80);
delay(75);tone(soundPin, 580,80);
delay(175);tone(soundPin, 660,80);
delay(75);tone(soundPin, 500,80);
delay(150);tone(soundPin, 430,80);
delay(75);tone(soundPin, 380,80);
delay(300);tone(soundPin, 660,100);
delay(75);tone(soundPin, 660,100);
delay(150);tone(soundPin, 660,100);
delay(150);tone(soundPin, 510,100);
delay(50);tone(soundPin, 660,100);
delay(150);tone(soundPin, 770,100);
delay(225);tone(soundPin, 380,100);
//tells the user it is over and delays it a little before going to the top again
delay(1000);
tone(soundPin, 440,200);
delay(200);
delay(200);
tone(soundPin, 440,400);
delay(200);
delay(200);
delay(5000);
}

void coinNoises() {
  tone(soundPin, B5, 75);delay(75);
  tone(soundPin, E6, 320);delay(320);
   tone(soundPin, B5, 75);delay(75);
  tone(soundPin, E6, 320);delay(320);
}

void powerUp()
{
   tone(soundPin, G4, 75);delay(75);
   tone(soundPin, B4, 75);delay(75);
   tone(soundPin, D5, 75);delay(75);
   tone(soundPin, G5, 75);delay(75);
   tone(soundPin, B5, 75);delay(75);
   
   tone(soundPin, Ab4, 75);delay(75);
   tone(soundPin, C5, 75);delay(75);
   tone(soundPin, Eb5, 75);delay(75);
   tone(soundPin, Ab5, 75);delay(75);
   tone(soundPin, Eb6, 75);delay(75);
   
   tone(soundPin, Bb4, 75);delay(75);
   tone(soundPin, D5, 75);delay(75);
   tone(soundPin, F5, 75);delay(75);
   tone(soundPin, Bb5, 75);delay(75);
   tone(soundPin, D6, 75);delay(75);
}
