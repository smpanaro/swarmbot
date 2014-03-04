#define red_pin 2
#define blue_pin 3
#define color_sensor_pin A1

#define pwm_rate 255 // 0 to 255
int red_baseline = 0;
int blue_baseline = 0;

int blue_prev = 0;
int red_prev = 0;


#define read_delay 1000


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
  

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
  detectColor(getRedLedValue(), getBlueLedValue(), 100);
}

void detectColor(int red, int blue, int threshold) {
  if ((red - blue) > threshold) Serial.println("red");
  else if ((blue - red) > threshold) Serial.println("blue");
  else Serial.println("black");
}

boolean onBlue() {
  int red = getRedLedValue();
  delay(10);
  int blue = getBlueLedValue();
  //Serial.print("[onBlue()]:\n");Serial.print("red: ");Serial.print(red);Serial.print("  red prev: ");Serial.println(red-red_prev);
  //Serial.print("blue: ");Serial.print(blue);  Serial.print("  blue prev: ");Serial.println(blue-blue_prev);
  red_prev = red;
  blue_prev = blue;
  return false; //(abs(red-blue) > dark_cal_threshold) && (blue > red);
}

boolean onRed() {
  int red = getRedLedValue();
  int blue = getBlueLedValue();
  Serial.print("[onRed()]:\n");Serial.print("red: ");Serial.print(red);Serial.print("\nblue: ");Serial.println(blue);
  return false; //(abs(red-blue) > dark_cal_threshold) && (red > blue);
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
