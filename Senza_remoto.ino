
//https://www.youtube.com/watch?v=7gjJfJgi6uE

/*  Includo librerie necessarie   */

//Queste sono utilizzate per la gestione del WiFi e della comunicazione con server Mqtt
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
//Queste sono utilizzate dal trasduttore di temperatura DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>


/*   Inizializzo e definisco le variabili del programma   */

#define ONE_WIRE_BUS D2                  // Definiamo il PIN dove sara' collegato il filo che si occupera' della misura della temperatura
OneWire oneWire(ONE_WIRE_BUS);          // Crea un'istanza "oneWire" per la comunicazione con altri "slaves"
DallasTemperature sensors(&oneWire);    // Passa la variabile "oneWire" alla liberia Dallas Temperature. 

const char* mqttServer = "192.168.232.58"; //Inserire IP del server 
const int mqttPort = 1883;                  //Inserire porta in ascolto sul server

WiFiClient espClient;
PubSubClient client(espClient);

/*   SETUP    */
void setup() {
  Serial.begin(115200);                                  //Inizializzo monitor seriale
  WiFiManager wifiManager;  //Inizializzazione locale, una volta configurata la prima volta non sara' necessario ripetere
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
  
  /*In questo passaggio si tenta la connessione al broker che continuera' a ciclare finche' non sara' stabilita la connessione*/
  while (!client.connected()){
    Serial.println("Connessione al broker MQTT...\n");
    if (client.connect("ESP8266Client" )) {
      Serial.println("Connessione al Broker RIUSCITA!.");
      client.publish("data",">>>Qui e' ESP8266<<<");
      }else{
        Serial.print("Connessione fallita con stato:rc ");
        Serial.print(client.state());
        delay(2000);
      }
    }
}


void loop(){
    client.loop();
    sensors.requestTemperatures();                  // Invia il comando per ricevere la temperatura
    float tempC = sensors.getTempCByIndex(0);       //Una volta fatta la lettura la salva nella variabile tempC
    Serial.println("Temperatura : " +String(tempC));//SerialPrint temperatura
    String toSend = String(tempC);                  //Conversione in stringa della variabile tempC e assegnazione in "toSend"
    client.publish("data","Temperatura:");          //Pubblicazione nel topic "data"
    client.publish("data",toSend.c_str());          //Pubblish-> topic="data", message="toSend"
    delay(1000);
}
