
#include <Wire.h>//Libreria de uso del protocolo I2C
#include "SSD1306Wire.h"//Libreria para el uso de la pantalla oled 
//Documentacion https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "DHT.h"//Libreria estandar de DHT
#include "MQ135Sensor.h" //Libreria desarrollada a medida
#define MQ135PIN 36  // Sensor MQ135
#define NH4 (0) // Tipo de gas a medir - Amonio
#define AudioOut 34//Pin de salida del Audio
#define AudioGain 33//Pin de configuración de la ganancia

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

//Variables usadas en el sistema de audio
const int sampleWindow = 250; // Sample window width in mS (50 mS = 20Hz) 
unsigned int sample; 
double dBAverage=0;
float en=0; 
int count= 0,r=0;
int avance=0;

//Variables para almacenar los resultados
float number1 = 0;//PM2.5
float number2 = 0;//Intensidad del sonido
float number3 = 0;//Temperatura
float number4 = 0;//Humedad
float number5 = 0;//Gas Amonio

//Funcion usada para medir la intensidad del sonido
float intensidad_sonido(void);


void setup() {
   Serial.begin(115200);
   Serial.println("Sistema de medición de calidad del Aire");
   Serial.println("Comfamiliar Pereira");
   Serial.println("César Augusto Álvarez Gaspar");
   dht.begin();
   pinMode(AudioGain,OUTPUT);
   digitalWrite(AudioGain,HIGH); 
   display.init();
   display.setContrast(255);
   display.setTextAlignment(TEXT_ALIGN_LEFT);
   display.setFont(ArialMT_Plain_16);
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
 number2=intensidad_sonido();
 number3=dht.readTemperature();
 number4=dht.readHumidity();
 number5 = MQ135_nh4.MQGetGasPercentage(mQ135Ro); // Lectura Amoniaco
 Consola="";
 Consola+=number2;
 Consola+=" ";
 Consola+=number3;  
 Consola+=" ";
 Consola+=number4;
 Consola+=" ";
 Consola+=number5;  
 Serial.println(Consola);
 display.clear();
 display.drawStringMaxWidth(0, 0,128, Consola);
 display.display();
   

  delay(1000);

}

float intensidad_sonido(void)
{
  unsigned int peakToPeak = 0; // peak-to-peak level 
  unsigned int signalMax = 0; 
  unsigned int signalMin = 1024; 
  unsigned int sample; 
  unsigned long startMillis= millis();
  for(count=0,dBAverage=0;count<10;count++)
  {
    while (millis() - startMillis < sampleWindow)
    { 
      sample = analogRead(AudioOut);
      //Serial.println(sample); 
      if (sample < 1024) 
      { // toss out spurious readings 
        if (sample > signalMax)
        { 
          signalMax = sample; // save just the max levels 
        } 
        else if (sample < signalMin) { 
          signalMin = sample; // save just the min levels 
        } 
      } 
    }
    peakToPeak = signalMax - signalMin; // max - min = peakpeak amplitude 
    double volts = (peakToPeak * 10.0) / 1024; // convert to volts 
    double dBW; 
    dBW = 20*log10(volts*20)+40; 
    dBAverage+=dBW;
  }
  en=dBAverage/count;
  return en; 
}
