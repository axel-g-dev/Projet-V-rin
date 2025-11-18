# **Projet – Vérin**

##  **Objectif du diaporama**

Présenter le fonctionnement complet du système de mesure et de commande du vérin, ainsi que les avancées du projet.


###  **Contenu du diaporama**

* **Présentation du capteur de distance**
  Description du principe de fonctionnement et des caractéristiques techniques.

* **Présentation du convertisseur ADS1115**
  Rôle du convertisseur 16 bits dans l’amélioration de la précision de mesure.
  Comparaison avec la conversion directe via l’ESP32.

* **Présentation du code**
  Structure du programme, acquisition des données et traitement du signal.

* **Résultats obtenus**
  Analyse des mesures et affichage graphique de l’évolution de la distance à l’aide de **Chart.js**
  -> [https://www.chartjs.org/](https://www.chartjs.org/)

* **Commande du vérin**
  Utilisation du **pont en H L298N** comme variateur pour piloter le vérin.

* **Problèmes rencontrés sans ADS1115**
  Limites de résolution et perte de précision avec la conversion intégrée de l’ESP32.


