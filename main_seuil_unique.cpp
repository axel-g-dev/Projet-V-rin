#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

// Adafruit 
Adafruit_ADS1115 ads;
float Voltage = 0.0;
float Distance; 

// Moteur du vérin sur le L298N
int motor1Pin1 = 17; 
int motor1Pin2 = 16; 
int enable1Pin = 18; 

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

void setup(void) 
{
Serial.begin(9600); 
if (!ads.begin(0x48)) {
  Serial.println("Echec d'initialisation de l'ADS1115.");
  while (1);
}
ads.setGain(GAIN_ONE);  // Plage ±4.096V pour mesurer 3.3V

/* Palge de de tension en fonction du gain
GAIN_TWOTHIRDS	6.144V	±6.144V	Tension = (adc × 6.144) / 32767
GAIN_ONE	4.096V	±4.096V	Tension = (adc × 4.096) / 32767
GAIN_TWO	2.048V	±2.048V	Tension = (adc × 2.048) / 32767
GAIN_FOUR	1.024V	±1.024V	Tension = (adc × 1.024) / 32767
GAIN_EIGHT	0.512V	±0.512V	Tension = (adc × 0.512) / 32767
GAIN_SIXTEEN	0.256V	±0.256V	Tension = (adc × 0.256) / 32767
*/

Serial.println("ADS1115 initialise avec GAIN_ONE (±4.096V)");
 pinMode(motor1Pin1, OUTPUT);
 pinMode(motor1Pin2, OUTPUT);

  // configure LEDC PWM (channel 0 on pin 18)
  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);

  Serial.print("Testing DC Motor...");
}

void loop(void) 
{
int16_t numCapteur;

numCapteur = ads.readADC_SingleEnded(0); // Lire la valeur du canal en mode commun

Voltage = (numCapteur* 4.096) / 32767; // formule à modifier en fonction du gain choisi

Serial.print("AIN0: "); 
Serial.println(numCapteur);
Serial.print("Voltage: ");
Serial.println(Voltage, 7); 

Serial.println("Valeur numérique :");
Serial.println(numCapteur);
Serial.println("Distance (cm) :");  
// Équation du système : 62,9 + -0,0145x + 1,65E-06x^2 + -9,32E-11x^3 + 2,05E-15x^4
  Distance = 62.9
                    - 0.0145 * numCapteur
                    + 1.65e-06 * pow(numCapteur, 2)
                    - 9.32e-11 * pow(numCapteur, 3)
                    + 2.05e-15 * pow(numCapteur, 4);

Serial.println(Distance);
Serial.println("---");

  // consigne 
  float marge = 0;
  float objectif = 12.0; 
   
  if (Distance < objectif - marge) {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
    ledcWrite(pwmChannel, 255);
  }
  else if (Distance > objectif + marge) {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(pwmChannel, 255);
  }
  else {
    ledcWrite(pwmChannel, 0);
  }

  delay(50);
}

// AU Départ mettre une consigne 




