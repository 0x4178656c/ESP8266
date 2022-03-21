
/*  Includo librerie necessarie   */

//Queste sono utilizzate per la gestione del WiFi e della comunicazione con server Mqtt
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <DHT.h>
#include <PubSubClient.h>
//Queste sono utilizzate dal trasduttore di temperatura DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>


/*   Inizializzo e definisco le variabili del programma   */

#define LED D6                          //Definisco pin uscita per pilotaggio remoto
#define ONE_WIRE_BUS D2                  // Definiamo il PIN dove sara' collegato il filo che si occupera' della misura della temperatura
OneWire oneWire(ONE_WIRE_BUS);          // Crea un'istanza "oneWire" per la comunicazione con altri "slaves"
DallasTemperature sensors(&oneWire);    // Passa la variabile "oneWire" alla liberia Dallas Temperature. 

//unsigned long previousMillis = 0;       //Variabili temporanee per utilizzo funzione millis()
//unsigned long interval = 1000;

const char* mqttServer = "192.168.232.58"; //Inserire IP del server 
const int mqttPort = 1883;                  //Inserire porta in ascolto sul server

WiFiClient espClient;
PubSubClient client(espClient);

/*   SETUP    */
void setup() {
  Serial.begin(115200);                                  //Inizializzo monitor seriale
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
                                                     //Inizializzazione locale, una volta configurata la prima volta non sara' necessario ripetere
  WiFiManager wifiManager;
  //wifiManager.resetSettings();                        //Decommentare per resettare le configurazioni salvate
  /*In caso di connessione assente, esp8266 si occupera' di aprire un AP dove configurare la rete.
    SSID:       WemosD1R2.conf
    IP_Portale: http://192.168.4.1 */
  wifiManager.autoConnect("WemosD1R2.conf");  

  //Se arriviamo a questo punto vorra' dire che la connessione e' avvenuta con successo
  Serial.println("Connessione al WiFi!...RIUSCITA.");
  sensors.begin();                                      //Inizializzo il sensore
  sensors.setResolution(11);                            //Setting risoluzione del sensore
  client.setServer(mqttServer, mqttPort);               //All'oggetto setServer gli passo le variabili definite sopra
  client.setCallback(callback);
  while (!client.connected()){
    Serial.println("Connessione al broker MQTT...\n");
    if (client.connect("ESP8266Client" )) {
      Serial.println("Connessione al Broker RIUSCITA!.");
      client.publish("data",">>>Qui e' ESP8266<<<");
      client.subscribe("LED");                        //Sottoscrizione al topic "LED", per controllo remoto

      }else{
        Serial.print("Connessione fallita con stato:rc ");
        Serial.print(client.state());
        delay(2000);
      }
    }
}

void loop(){
    client.loop();
    sensors.requestTemperatures(); // Invia il comando per ricevere la temperatura
    float tempC = sensors.getTempCByIndex(0);
    Serial.println("Temperatura : " +String(tempC));
    String toSend = String(tempC);
    client.publish("data","Temperatura:");
    client.publish("data",toSend.c_str());
    delay(1000);
}


/*NB: Funzione callback utilizzata per verificare se arriva un comando dal broker.
  il topic scelto per questo condice e':"LED", con un unico messaggio accettato: "LED ON" e relativo output.
  EX: moscquitto_sub -h [IP] -t 'LED' -m 'LED ON'
  
  */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Messaggio arrivato nel topic: ");
  Serial.println(topic);

  Serial.print("Message:");
    Serial.println();  
  String message = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message+=(char)payload[i];
  }
  Serial.println("-----------------------");
  if(String(topic)=="LED"){
    if(message=="LED ON"){
      digitalWrite(LED,HIGH);
      Serial.println("LED IS ON");
    }
    else{
      digitalWrite(LED,LOW);
    }
  }
    
}
