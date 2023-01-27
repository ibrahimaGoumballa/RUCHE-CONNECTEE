/*
  First Configuration
  This sketch demonstrates the usage of MKR WAN 1300/1310 LoRa module.
  This example code is in the public domain.
*/
#include <MKRWAN.h>
//#include <dht.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "HX711.h"

#include <Wire.h>
#include "DFRobot_INA219.h"

// PIN DHT22
#define DHTPIN3 4
#define DHTPIN4 5 
#define DHTTYPE    DHT22
DHT dht1(DHTPIN3, DHTTYPE);
DHT dht2(DHTPIN4, DHTTYPE);

// WEIGHT SENSOR 
#define LOADCELL_DOUT_PIN  0
#define LOADCELL_SCK_PIN  1
HX711 scale;
float calibration_factor = 13350;

// LIGHT SENSOR
DFRobot_INA219_IIC     ina219(&Wire, INA219_I2C_ADDRESS4);
float ina219Reading_mA = 1000;
float extMeterReading_mA = 1000;

//DSP SENSOR
#define ONE_WIRE_BUS 7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer1, insideThermometer2;



//OneWire ds18x20[]= { 6, 7 };
//const int oneWireCount = sizeof(ds18x20) / sizeof (OneWire);
//DallasTemperature sensor [oneWireCount];



LoRaModem modem;


// VARIABLE TTN
String appEui ;
String appKey ;
String devAddr;
String nwkSKey;
String appSKey;

void setup() {
  
   Serial.begin(9600);
  

   // Start up the library on all defined bus-wires
  DeviceAddress deviceAddress;


  delay(1000);//Wait before accessing Sensor

  //TAR WEIGHT SENSOR
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  //scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); 
  scale.set_scale(calibration_factor);
  

  //*********ALLUMAGE DE LA LED AU DEMARRAGE*****************//
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(10000);                       // wait for 10 seconds
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  
 //*********CONNECTION TO TTN*****************//

  if (!modem.begin(EU868)) {
    while (1) {}
  };
  if (modem.version() != ARDUINO_FW_VERSION) {
  }
   int mode = 1;
   int connected;
  
  if (mode == 1) {
    
    appEui = "0000000000000000";
    appKey = "0EDEB1AC7F6DA863DF7072DD8EAE2449";
    appKey.trim();
    appEui.trim();
    connected = modem.joinOTAA(appEui, appKey);
  } else if (mode == 2) {

    while (!Serial.available());
    devAddr = Serial.readStringUntil('\n');

    while (!Serial.available());
    nwkSKey = Serial.readStringUntil('\n');

    while (!Serial.available());
    appSKey = Serial.readStringUntil('\n');

    devAddr.trim();
    nwkSKey.trim();
    appSKey.trim();

    connected = modem.joinABP(devAddr, nwkSKey, appSKey);
  }

  if (!connected) {
    while (1) {}
  }
  
  delay(5000);
 

}

void loop() {
  
  //***********DECLARATION VARIABLE  ***************//
  //DHT22 INTERRIEUR
  dht1.begin();
   int valeur = analogRead(A3); // Mesure la tension sur la broche A0
   float hum = dht1.readHumidity();
   float temp3 = dht1.readTemperature();
   short h = (short)(hum*100);
   short t = (short)(temp3*100);

   // DHT22 EXTERRIEUR
  dht2.begin(); 
   float h2 = dht2.readHumidity();
   float t2 = dht2.readTemperature();
   short h_i = (short)(h2*100);
   short t_i = (short)(t2*100);
   
   // Panneau solaire 
   while(ina219.begin() != true) {
        //Serial.println("INA219 begin faild");
        delay(2000);
    }
    ina219.linearCalibrate(ina219Reading_mA, extMeterReading_mA);


   //VARIABLE POUR MESURER BDSP19B20 //
   sensors.begin();
   sensors.getAddress(insideThermometer1, 0);
   sensors.getAddress(insideThermometer2, 1);
   sensors.requestTemperatures();
   float temperature0 =  sensors.getTempC(insideThermometer1);
   float temperature1 = sensors.getTempC(insideThermometer2);
  
   short t_0 = (short)(temperature0*100);
   short t_1 = (short)(temperature1*100);

//VARIABLE POUR MESURER LE POIDS //
   float poid = (scale.get_units()/2.2046)-4.4;
   short weight_kg = (short)(poid*100);

 // VARIABLE LIGHT//
  //short pancourant =(short)(ina219.getCurrent_mA()*100);
  short l =(short)(ina219.getPower_mW()*100);
  
// VARIABLE POUR MESURER % BATTERIE//
float tension = valeur * (5.79 / 929.0); // Transforme la mesure (nombre entier) en tension via un produit en croix, 1024 = Valeur max pour 5.79 du convertisseur
float Pourcent=(tension/5.79)*100; 

  short bv = (short)(Pourcent*100);

 
// ENVOIE DES DONNEES SUR TTN//
   modem.setPort(3);
  modem.beginPacket();

  modem.write(h);
 modem.write(t_0);
 modem.write(h_i);
 modem.write(t_i);
 modem.write(t);
 modem.write(t_1);
 modem.write(weight_kg);
 modem.write(bv);


// modem.write(pancourant);
 modem.write(l);

//VERIFICATION ERREUR //
    int err;
  err = modem.endPacket(true);
   //err = modem1.endPacket(true);
  if (err > 0) {
    //Serial.println("Message sent correctly!");
    
    pinMode(LED_BUILTIN, OUTPUT);
digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(5000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);
  
  } else {
    //Serial.println("Error sending message :(");

    pinMode(LED_BUILTIN, OUTPUT);
digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);
  }


// MODIFIER TEMPS DE CYCLE D'ENVOIE //  
delay(120000);
  while (modem.available()) {

delay(120000); // Attendre 5sec

 Serial.write(modem.read());
          
  }
   
 modem.poll(); 
}
