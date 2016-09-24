//
// Gestion de la consigne de chauffage
// Lit la consigne en °C sur le pin V1
// Remonte la température sur V2
// et le taux d'humidite sur V3
//
#define BLYNK_PRINT Serial    // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SimpleTimer.h>

#include "DHT.h"      //Librairie DHT

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "xxx";

WidgetLED led_on(V4);
WidgetLCD lcd(V0);

#define DHTPIN D4    // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 

#define CommutThermPin D1

//Liste des écarts de température
const int ecartMax = 5;
const int ecartMoy = 3;
const int ecartMin = 1;

//Consigne par defaut
int consigne = 18;

// Par défaut la tempo est 1 minute
long tempo = 60L;
//Valeur initiale de l'attente
long attente = 0L;

int temperature;

SimpleTimer timer;
int level = LOW;

//On initialise le DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(CommutThermPin, OUTPUT);
  dht.begin();
  Blynk.begin(auth, "xxx", "xxx", IPAddress(xx,xx,xx,xx));

  //On verifie le besoin de chauffage toutes les secondes
  timer.setInterval(1000L, checkChauffage);
 }

void loop() {
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
}

//// Set Consigne
// This function will be called every time
// when App writes value to Virtual Pin 1
BLYNK_WRITE(V1)
{
  BLYNK_LOG("Nouvelle consigne: %s", param.asStr());
  // You can also use:
  // int i = param.asInt() or
  // double d = param.asDouble()
  consigne = param.asInt();
}

BLYNK_CONNECTED() {
    Blynk.syncAll();
}

//On calcul la consigne de chauffage
void checkChauffage() {
  printTemp();
  temperature = dht.readTemperature();
  if (isnan(temperature)) {
    BLYNK_LOG("Erreur calcul de temperature");
    return;
  }
  
  if (attente < 0) {
    if (temperature < consigne - ecartMax) {
      level = HIGH;
      attente = tempo * 5;
    }
    else if (temperature < consigne - ecartMoy) {
      level = HIGH;
      attente = tempo * 3;
    }
    else if (temperature <= consigne - ecartMin) {
      level = HIGH;
      attente = tempo;
    }
    else {
      level = LOW;
      attente = tempo;
    }
    if (level == HIGH) {
      led_on.on(); //chauffage ?
    } else {
      led_on.off();
    }

  }
  digitalWrite(CommutThermPin, level);
  attente = attente - 1L;
  lcd.print(0, 0, "Attente :");
  lcd.print(11,0,attente);
}

void printTemp() {
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    BLYNK_LOG("Failed to read from DHT sensor!");
    lcd.print(0,1,"Erreur DHT");
    return;
  }
  lcd.print(0,1,"                   ");

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);

  Blynk.virtualWrite(V2,t); //temp
  Blynk.virtualWrite(V3,h); //humidity

  BLYNK_LOG("consigne : %d", consigne);
  BLYNK_LOG("temp : %d", (int)t);
  BLYNK_LOG("hum : %d", (int)h);
  BLYNK_LOG("attente : %d", attente);
  BLYNK_LOG("level : %d", level);
  
}


