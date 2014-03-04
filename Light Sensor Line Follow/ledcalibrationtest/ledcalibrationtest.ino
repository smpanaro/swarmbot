#define red_pin 2
#define blue_pin 3
#define sensor_pin A1

#define pwm_rate 255 // 0 to 255


#define read_delay 1000


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

}

// the loop routine runs over and over again forever:
void loop() {
  analogWrite(red_pin, 0);
  analogWrite(blue_pin, 0);
  
  delay(read_delay);
  
  Serial.print("dark cal: "); Serial.println(analogRead(sensor_pin));
  
    delay(read_delay);
  
  analogWrite(red_pin, pwm_rate);
  analogWrite(blue_pin, 0);
  
  delay(read_delay);
  
  Serial.print("red led: "); Serial.println(analogRead(sensor_pin));
  
    delay(read_delay);
  
  analogWrite(red_pin, 0);
  analogWrite(blue_pin, pwm_rate);
  
  delay(read_delay);
  
  Serial.print("blue led: "); Serial.println(analogRead(sensor_pin));
  
    delay(read_delay);
  
  analogWrite(red_pin, 0);
  analogWrite(blue_pin, 0);

  delay(read_delay);
  
  Serial.println();
  
  delay(500);
}
