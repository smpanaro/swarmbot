#define red_pin 2
#define blue_pin 3
#define color_sensor_pin A1

//Motor Constants
#define M1_HIGH_PIN 12
#define M1_LOW_PIN 11
#define M2_HIGH_PIN 7
#define M2_LOW_PIN 8

#define pwm_rate 255 // 0 to 255
int red_baseline = 0;
int blue_baseline = 0;

int blue_prev = 0;
int red_prev = 0;


#define read_delay 1000
#define spd 255
#define time 20

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  pinMode(M1_HIGH_PIN, OUTPUT);
  pinMode(M1_LOW_PIN, OUTPUT); 
  pinMode(M2_HIGH_PIN, OUTPUT);
  pinMode(M2_LOW_PIN, OUTPUT);

  for (int i = 0; i < 3; i++) {
    red_baseline += getRedLedValue();
    blue_baseline += getBlueLedValue();
  } 
  red_baseline /= 3;
  blue_baseline /= 3;
  Serial.print("red base: ");Serial.println(red_baseline);
  Serial.print("blue base: ");Serial.println(blue_baseline);
}

// the loop routine runs over and over again forever:
void loop() {
  int a = detectColor(getRedLedValue(), getBlueLedValue(), 100);
  if(a == 0){
    Serial.println("left");
    yaw_left(100);
    delay(time);
  }
  else{
    Serial.println("right");
    yaw_right(100);
    delay(time);
  }



}

int detectColor(int red, int blue, int threshold) {
  int temp;
  if ((red - blue) > threshold) {
    Serial.println("red");
    return 1;
  }
  else if ((blue - red) > threshold){
     Serial.println("blue");
     return 2;
  }
  Serial.println("black");
  return 0;
}

int getRedLedValue() {
  analogWrite(red_pin, pwm_rate);
  analogWrite(blue_pin, 0);
  delay(100);
  return analogRead(color_sensor_pin);
}

int getBlueLedValue() {
  analogWrite(red_pin, 0);
  analogWrite(blue_pin,255);
  delay(100);
  return analogRead(color_sensor_pin);
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

void yaw_left(int speed){
    setHighPin(M1_HIGH_PIN, M1_LOW_PIN, speed);
    setHighPin(M2_LOW_PIN, M2_HIGH_PIN, 0);
  
}

void yaw_right(int speed){
    setHighPin(M1_LOW_PIN, M1_HIGH_PIN, 0);
    setHighPin(M2_LOW_PIN, M2_HIGH_PIN, speed);
    
}

void turn(char dir, int angle){
  if(dir == 'l'){
    left(255);
    delay(1200*(angle/90));
    stop();
    delay(200);
  }
    else if(dir == 'r'){
    right(255);
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
