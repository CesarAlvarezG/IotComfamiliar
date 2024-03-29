
#include <Wire.h>
#include "SSD1306Wire.h"
#include "DHT.h"
#include "MQ135Sensor.h"
#define MQ135PIN 39  // Sensor MQ135
#define NH4 (0) // Tipo de gas a medir - Amonio

#define DTHPIN 18
#define DTHTYPE DHT11

DHT dht(DTHPIN, DTHTYPE);
SSD1306Wire display(0x3c, SDA, SCL);
mq135sensor MQ135_nh4(MQ135PIN,NH4); // Objeto del sensor MQ135

const String Bienvenida="Sistema de medición de calidad del Aire";
String Consola="";
float h=0;//Humedad
float t=0;//Temperatura
float mQ135Ro; // Resistencia de calibracion para el MQ135
float lec_nh4 = 0;


void setup() {
   Serial.begin(115200);
   Serial.println("Sistema de medición de calidad del Aire");
   Serial.println("Comfamiliar Pereira");
   Serial.println("César Augusto Álvarez Gaspar");
   dht.begin();
   display.init();
   display.setContrast(255);
   display.setTextAlignment(TEXT_ALIGN_LEFT);
   display.setFont(ArialMT_Plain_24);
   display.display();
   display.drawStringMaxWidth(0, 0,128, Bienvenida);
   display.display();
   //  MQ135 inicializacion
  Serial.println("Calibrando... \n");
  mQ135Ro = MQ135_nh4.MQCalibracion();
  Serial.print("Calibracion completa \n \n");
  Serial.print("MQ135 Ro=");
  Serial.print(mQ135Ro);
  Serial.println("kohm");
  delay(2000);
}

// the loop function runs over and over again forever
void loop() {
 h=dht.readHumidity();
 t=dht.readTemperature();
 lec_nh4 = MQ135_nh4.MQGetGasPercentage(mQ135Ro); // Lectura Amoniaco
 Consola="";
 Consola+=h;
 Consola+=" ";
 Consola+=t;  
 Consola+=" ";
 Consola+=lec_nh4;  
 Serial.println(Consola);
 
 static uint16_t start_at = 0;
  display.clear();
  uint16_t firstline = display.drawStringMaxWidth(0, 0, 128, Consola.substring(start_at));
  display.display();
  if (firstline != 0) {
    start_at += firstline;
  } else {
    start_at = 0;
    delay(1000); // additional pause before going back to start
  }
  delay(1000);

}
