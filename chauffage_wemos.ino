#include "Timer.h"	//from https://github.com/JChristensen/Timer
#include "DHT.h"	//from https://github.com/adafruit/DHT-sensor-library
#include "ESP8266WiFi.h"


#define DHTPIN D4
#define DHTTYPE DHT11
#define CommutThermPin 0//pin on-off

const char* ssid = "*";
const char* password = "***";

//Declaration du serveur wifi
WiFiServer server(80);

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
int level = LOW;
Timer t;

//On initialise le DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  pinMode(CommutThermPin, OUTPUT);
  dht.begin();

  //On verifie le besoin de chauffage toutes les secondes
  t.every(1000, checkChauffage);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
  
 }

void loop() {
  t.update();

  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    manageClient(client);
  }
}

//On calcul la consigne de chauffage
void checkChauffage() {
  printTemp();
  temperature = dht.readTemperature();
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
  }
  digitalWrite(CommutThermPin, level);
  attente = attente - 1L;
}

String printTemp() {
    // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return "";
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.computeHeatIndex(f, h);

  Serial.print(consigne); // consigne
  Serial.print("&");
  Serial.print(attente);  // attente
  Serial.print("&"); 
  Serial.print(h);  // tx humidite
  Serial.print("&");
  Serial.print(t);  // temperature relevée
  Serial.print("&");
  Serial.println(level);  // etat du chauffage

  String s = "Consigne :" ;
  s += consigne;
  s +=  "<br/>";
  s += "Attente :";
  s += attente;
  s += "<br/>";
  s += "Humidite :";
  s += h;
  s += "<br/>";
  s += "Temperature :";
  s += t;
  s += "<br/>";
  s += "Chauffage :";
  s += level;
  s += "<br/>";

  return s;
}

void manageClient(WiFiClient client) {
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int val;
  String sTemp;
  Serial.println(req);
  if (req.indexOf("GET /temp/") != -1 && req.length() > 20) {
    sTemp = req.substring(10,req.length()-8);
    consigne = sTemp.toInt();
    Serial.print("Temperature demandée: ");
    Serial.println(consigne);
  } else if (req.indexOf("GET /temp") != -1 && req.length() == 18) {
    // rien de particulier
  } else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nTemperature set to ";
  s += consigne;
  s += "<br>";
  s += printTemp();
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

