#include <Arduino.h>

// Motor A
int motor1Pin1 = 17; 
int motor1Pin2 = 16; 
int enable1Pin = 18; 

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

void setup() {
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);

  Serial.begin(115200);

  // configure LEDC PWM (channel 0 on pin 18)
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);

  Serial.print("Testing DC Motor...");
}

void loop() {

  // Move Forward (max speed)
  Serial.println("Moving Forward");
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH);
  ledcWrite(pwmChannel, 255); 
  delay(2000);

  // Stop
  Serial.println("Motor stopped");
  ledcWrite(pwmChannel, 0);
  delay(1000);

  // Move Backward
  Serial.println("Moving Backwards");
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(pwmChannel, 255);
  delay(2000);

  // Stop
  Serial.println("Motor stopped");
  ledcWrite(pwmChannel, 0);
  delay(1000);

  // Increase speed
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  while (dutyCycle <= 255) {
    ledcWrite(pwmChannel, dutyCycle);
    Serial.print("Forward with duty cycle: ");
    Serial.println(dutyCycle);
    dutyCycle = dutyCycle + 5;
    delay(500);
  }

  dutyCycle = 200;
}
