## 1. Description Générale
Système d’asservissement de position pour un actionneur linéaire (vérin). Le système lit la position via un capteur analogique (ADS1115), la filtre, calcule l'erreur par rapport à une consigne reçue via une interface Web, et pilote le moteur via un pont en H (L298N) avec une régulation proportionnelle.

---

## 2. Matériel et Connexions

**Microcontrôleur :** ESP32
**Communication :** I2C (pour ADS1115) et PWM (pour L298N).

| Composant | Pin ESP32 | Fonction | Notes |
| :--- | :--- | :--- | :--- |
| **L298N** | GPIO 17 | IN1 | Direction A |
| **L298N** | GPIO 16 | IN2 | Direction B |
| **L298N** | GPIO 18 | ENA | PWM (Vitesse) |
| **ADS1115** | SDA (21) | SDA | I2C Data |
| **ADS1115** | SCL (22) | SCL | I2C Clock |



---

## 3. Configuration Logicielle

### Bibliothèques requises
* `Adafruit_ADS1X15` (Adafruit)
* `WiFi` (Standard ESP32)
* `WebServer` (Standard ESP32)

### Paramètres Réseau
* **SSID :** `raf`
* **Password :** `Motdepasse12.`
* **IP :** DHCP (Affichée sur le port Série au démarrage).

### Paramètres PWM (LEDC)
* **Fréquence :** 30 kHz
* **Résolution :** 8 bits (0-255)
* **Canal :** 0

---

## 4. Fonctionnement Détaillé

### A. Acquisition et Traitement du Signal (Boucle Loop)
1.  **Lecture ADC :** Canal 0 (`AIN0`) de l'ADS1115. Gain `GAIN_ONE` (+/- 4.096V).
2.  **Conversion Physique :** Transformation de la valeur brute (16-bit) en centimètres via un polynôme de degré 4 :

$$Pos(x) = 62,9 - 0,0145x + 1,65 \cdot 10^{-6}x^2 - 9,32 \cdot 10^{-11}x^3 + 2,05 \cdot 10^{-15}x^4$$

**Format Code C++ :**

```cpp
position_actuelle = 62.9 - 0.0145 * numCapteur + 1.65e-06 * pow(numCapteur, 2) - 9.32e-11 * pow(numCapteur, 3) + 2.05e-15 * pow(numCapteur, 4);
```
3.  **Filtrage :** Moyenne glissante sur **10 échantillons**.
    * *Initialisation :* Moyenne cumulative jusqu'au remplissage du tableau.
    * *Régime établi :* Buffer circulaire, somme divisée par 10.

### B. Algorithme de Régulation (Asservissement)
Le système utilise une régulation proportionnelle avec zone morte.

* **Zone Morte (Trigger) :** `±0.2 cm`. Si l'erreur est inférieure, le moteur coupe (arrêt des oscillations).
* **Bande Proportionnelle :** `3.0 cm`. Zone où la vitesse varie linéairement.
* **Commande Moteur :**
    * *Erreur > 3.2 cm :* Vitesse Max (255).
    * *0.2 cm < Erreur < 3.2 cm :* Vitesse progressive de `pwm_min` (150) à `pwm_max` (255).
    * *Erreur < 0.2 cm :* Arrêt (0).



### C. Interface Web
Le serveur écoute sur le port **80**.

* **Page d'accueil (`/`) :**
    * Affiche la position actuelle (rafraîchissement HTML `meta refresh` toutes les 1s).
    * Affiche un slider (range input) de 0 à 60 cm.
* **Route de commande (`/set`) :**
    * Appelée lors de la validation du formulaire.
    * Récupère l'argument `c` (consigne).
    * Met à jour la variable globale `consigne`.
    * Redirige vers la page d'accueil.

---

## 5. Variables Ajustables
Pour modifier le comportement du système, ajuster ces constantes en début de code ou dans la `loop` :

* `consigne` : Valeur cible par défaut au démarrage (ex: 26.0).
* `taille_tableau` : Lissage des mesures (ex: 10).
* `bande_proportionnelle` : Douceur de l'approche (ex: 3.0).
* `pwm_min` : Tension minimale pour vaincre les frottements moteur (ex: 150).
