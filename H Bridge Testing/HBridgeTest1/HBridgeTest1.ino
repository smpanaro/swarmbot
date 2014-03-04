//pin 2 on the hbridge is connected to pin 12 on arduino
//pin 7 on hbridge connected to pin 11 on arduino
// pin 11 and 12 on the arduino control motor 1
// pin 15 on the hbridge is connected to pin 8 on arduino
// pin 10 on the hbridge is connected to pin 7 on arduino
// pin 7 and 8 on the arduino control motor 2

#define M1_HIGH_PIN 12
#define M1_LOW_PIN 11
#define M2_HIGH_PIN 7
#define M2_LOW_PIN 8

void setup(){
   pinMode(M1_HIGH_PIN, OUTPUT);
   pinMode(M1_LOW_PIN, OUTPUT); 
   pinMode(M2_HIGH_PIN, OUTPUT);
   pinMode(M2_LOW_PIN, OUTPUT);
}

void loop(){
  forward(100);  
  delay(1000);
  stop();
  delay(500);
  reverse(100);
  delay(1000);
  stop();
  delay(500);
  
  turn('l', 90);
  turn('r', 90);
  turn('l',180);
}

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
  if (speed > 255) return;
  
  digitalWrite(low_pin, LOW);
  analogWrite(high_pin, speed);
}

void setLowPin(int high_pin, int low_pin, int speed) {
  if (speed > 255) return;
  
  digitalWrite(high_pin, LOW);
  analogWrite(low_pin, speed);
}
