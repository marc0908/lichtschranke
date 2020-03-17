//TODO: LCD
#include <EEPROM.h>
#include <Encoder.h>

/*
 * EEPROM Speicher Konzept
 * Round 1 --> Address: 0; Value = TIME
 * Round 2 --> Address: 1; Value = TIME
 * ...
 * Round n --> Address: n-1; VALUE = TIME
 */

  
//Lichtschranken-Input-Pin (Button)
const int lsInput = 11;

//Start-Button-Pin (Button)
const int startPin = 12;

//DT-Pin (Rotary Encoder)
const int dtPin = 3;

//CLK-Pin (Rotary Encoder)
const int clkPin = 2;

//Alte Position des Rotary-Encoders
float newPos = 0;

//Neue Position des Rotary-Encoders
float oldPos = -999;

Encoder rotary(dtPin, clkPin);


volatile boolean TurnDetected;
volatile boolean rotationdirection;


//Mindestrunden
const int minRound = 1;

//Maximale Runden
int maxRound = 9;

//Rotary-Econder Position
int rotaryPosition = 0;

//Rotary-Encoder Entprellung
bool rotLock = false;
int lockTime = 0;

//Insg. Anzahl der Runden
const int anzRunden = 5;

//Derzeitige Runde
int currRound = 0;

//Lichtschranken Sperrzeit [SKEUNDEN]
const int sperrZeit = 2;

//Runden-Zeit
float roundTime = 0;

bool gestartet = false;
bool lsGesperrt = false;
bool setupMode = false;

//Millisekunden seit Start der Runde
float rundeStart = 0;

void setup() {
  pinMode(lsInput, INPUT);  
  pinMode(startPin, INPUT);
  pinMode(clkPin,INPUT);
  pinMode(dtPin,INPUT);  
  
  Serial.begin(1000000);

  //Beim Start wird gleich der Setup-Modus gestartet
  setupMode = true;

  rotary.write(minRound * 4);
  newPos = minRound * 4;
}

void loop() {
  if(setupMode){
    Serial.println(newPos);

    newPos = rotary.read() / 4;  // Die "neue" Position des Encoders wird definiert. Dabei wird die aktuelle Position des Encoders über die Variable.Befehl() ausgelesen. 

    if (newPos != oldPos)  // Sollte die neue Position ungleich der alten (-999) sein...
    {
       oldPos = newPos;       
    }    
    
    if(newPos == (minRound - 1)){
      rotary.write(maxRound * 4);
    }
    else if(newPos == (maxRound + 1)){
      rotary.write(minRound * 4);
    }
  }else{
    if(digitalRead(startPin) == HIGH && !gestartet){
      //Spiel gestartet
      Serial.println("Messung gestartet!");
      gestartet = true;
    }

    if(gestartet){
      if(digitalRead(lsInput) == HIGH && !lsGesperrt){
        //Lichtschranken ausgelöst
        if(currRound != 0){
          //Speicher Rundenzeit in EEPROM
          roundTime = millis() - rundeStart;
          EEPROM.write(currRound - 1, roundTime);
    
          Serial.print("");
          Serial.print(currRound);
          Serial.print(",");
          Serial.print(roundTime);
          Serial.println();
          
          //Neue Runde Starten
          currRound++; 
  
          //Speichert Microsekunden beim Start der Runde
          rundeStart = millis();
        }
        else{
          rundeStart = millis();
  
          currRound = 1;
        }
  
        //Sperrt Lichtschranken für 5 sek.
        lsGesperrt = true;
  
        //Rundenzeit zurücksetzen
        roundTime = 0;
      } 
  
      //Prüfen ob 5 Sekundern vergangen sind
      if(millis() >= rundeStart + (sperrZeit * 1000) && lsGesperrt){
        //Lichtschranken enstperren
        lsGesperrt = false;
      }
  
      //Prüfen ob alle Runden abgefahren wurden
      if(currRound == anzRunden + 1){
        Serial.println("Messung Ende");
  
        gestartet = false;
      }
    }
  }
}
