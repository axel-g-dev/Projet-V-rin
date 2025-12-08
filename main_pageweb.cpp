#include <Arduino.h>
#include <math.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <WiFi.h>
#include <WebServer.h>

// --------------------------------------------
// WIFI
// --------------------------------------------
const char* ssid = "raf";
const char* password = "Motdepasse12.";

WebServer server(80);

// --------------------------------------------
// ADS1115
// --------------------------------------------
Adafruit_ADS1115 ads;
float Voltage = 0.0;
float position_actuelle;

// --------------------------------------------
// MOTEUR L298N
// --------------------------------------------
int motor1Pin1 = 17;
int motor1Pin2 = 16;
int enable1Pin = 18;

// PWM
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;

// --------------------------------------------
// VARIABLES DE COMMANDE
// --------------------------------------------
float consigne = 26.0;  // MODIFIÉE PAR PAGE WEB

unsigned long ancien_affichage = 0;

// --------------------------------------------
// MOYENNE GLISSANTE
// --------------------------------------------
const int taille_tableau = 10;
float tab_mesure[taille_tableau];
int index_tab = 0;
bool tableau_rempli = false;

// --------------------------------------------
// PAGE WEB
// --------------------------------------------
String pageWeb()
{
    String html = "<html><head><meta charset='UTF-8'>";
    html += "<meta http-equiv='refresh' content='1'>";
    html += "<style>";
    html += "body{font-family:Arial;text-align:center;margin-top:40px;}";
    html += "h2{color:#333;}";
    html += "input[type=range]{width:80%;}";
    html += "</style></head><body>";

    html += "<h2>⚙️ Contrôle du vérin</h2>";
    html += "<h3>Position actuelle : <b>" + String(position_actuelle, 1) + " cm</b></h3>";

    html += "<form action='/set'>";
    html += "<input type='range' min='0' max='60' value='" + String(consigne) + "' name='c' oninput='v.value=c.value'>";
    html += "<br><output id='v'>" + String(consigne) + "</output> cm<br><br>";
    html += "<input type='submit' value='Valider' style='padding:10px 20px;'>";
    html += "</form>";

    html += "</body></html>";
    return html;
}

// --------------------------------------------
// SETUP
// --------------------------------------------
void setup(void)
{
  Serial.begin(9600);

  // --- WIFI ---
  WiFi.begin(ssid, password);
  Serial.print("Connexion au WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nConnecté !");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());

  // Routes Web
  server.on("/", []() {
      server.send(200, "text/html", pageWeb());
  });

  server.on("/set", []() {
      if (server.hasArg("c"))
      {
          consigne = server.arg("c").toFloat();
          Serial.print("Nouvelle consigne : ");
          Serial.println(consigne);
      }
      server.sendHeader("Location", "/");
      server.send(303);
  });

  server.begin();
  Serial.println("Serveur web disponible.");

  // --- ADS1115 ---
  if (!ads.begin(0x48))
  {
    Serial.println("Echec d'initialisation de l'ADS1115.");
    while (1);
  }
  ads.setGain(GAIN_ONE);

  // --- MOTEUR ---
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);
}

// --------------------------------------------
// LOOP
// --------------------------------------------
void loop(void)
{
  server.handleClient();  // *** Gestion du serveur web ***

  int16_t numCapteur = ads.readADC_SingleEnded(0);

  Voltage = (numCapteur * 4.096) / 32767;

  position_actuelle = 62.9
    - 0.0145 * numCapteur
    + 1.65e-06 * pow(numCapteur, 2)
    - 9.32e-11 * pow(numCapteur, 3)
    + 2.05e-15 * pow(numCapteur, 4);

  // --- MOYENNE GLISSANTE ---
  if (!tableau_rempli)
  {
      tab_mesure[(taille_tableau - 1) - index_tab] = position_actuelle;
      index_tab++;

      float somme = 0;
      for (int i = 0; i < index_tab; i++)
          somme += tab_mesure[i];

      position_actuelle = somme / index_tab;

      if (index_tab == taille_tableau)
      {
          tableau_rempli = true;
          index_tab = 0;
      }
  }
  else
  {
      tab_mesure[(taille_tableau - 1) - index_tab] = position_actuelle;
      index_tab++;

      float somme = 0;
      for (int i = 0; i < taille_tableau; i++)
          somme += tab_mesure[i];

      position_actuelle = somme / taille_tableau;

      if (index_tab == taille_tableau)
          index_tab = 0;
  }

  // --- CONTRÔLE MOTEUR ---
  float bande_proportionnelle = 3.0;
  float trigger = 0.2;
  int pwm_max = 255;
  int pwm_min = 150;
  float ordre_pwm;

  float erreur = consigne - position_actuelle;

  if (erreur > trigger)
  {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, HIGH);
  }
  else if (erreur < -trigger)
  {
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
  }
  else
  {
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
  }

  erreur = abs(erreur);

  if (erreur > bande_proportionnelle + trigger)
      ordre_pwm = pwm_max;
  else if (erreur > trigger)
      ordre_pwm = pwm_min + ((erreur - trigger) / bande_proportionnelle) * (pwm_max - pwm_min);
  else
      ordre_pwm = 0;

  ledcWrite(pwmChannel, int(ordre_pwm));


  // --- AFFICHAGE ---
  if (millis() > ancien_affichage + 200)
  {
    Serial.print("Position : ");
    Serial.print(position_actuelle);
    Serial.print("  | Consigne : ");
    Serial.print(consigne);
    Serial.print("  | PWM : ");
    Serial.println(ordre_pwm);

    ancien_affichage = millis();
  }
}
