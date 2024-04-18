#include <Wire.h>//Libreria de uso del protocolo I2C
#include "SSD1306Wire.h"//Libreria para el uso de la pantalla oled 
//Documentacion https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "DHT.h"//Libreria estandar de DHT
#include "MQ135Sensor.h" //Libreria desarrollada a medida
#include <Tomoto_HM330X.h>//Libreria del Sensor PM2.5 Azul
//Documentacion https://github.com/tomoto/Arduino_Tomoto_HM330X
//Librerias estandar para uso de Thingspeak
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#define MQ135PIN 36  // Sensor MQ135
#define NH4 (0) // Tipo de gas a medir - Amonio
#define AudioOut 34//Pin de salida del Audio
#define AudioGain 33//Pin de configuración de la ganancia
#define TypePM_A 0//El sensor PM2.5 azul
#define TypePM_B 1//El sensor PM2.5 negro
//Pines para lectura del sensor PMB
//Documentacion https://wiki.dfrobot.com/PM2.5_laser_dust_sensor_SKU_SEN0177
#define pinTxPMB 16
#define pinRxPMB 17
//Serial para el manejo del PM25B
#include<HardwareSerial.h>



#define DTHPIN 18
#define DTHTYPE DHT11

#include <WiFi.h>
DHT dht(DTHPIN, DTHTYPE);
SSD1306Wire display(0x3c, SDA, SCL);
mq135sensor MQ135_nh4(MQ135PIN,NH4); // Objeto del sensor MQ135
Tomoto_HM330X sensorPMA; //Objeto del sensor PM2.5 A
int TypePM = TypePM_A; //Define el tipo de sensor que se usara  

const String Bienvenida="Sistema de medición de calidad del Aire";
String Consola="";
float h=0;//Humedad
float t=0;//Temperatura
float mQ135Ro; // Resistencia de calibracion para el MQ135
float lec_nh4 = 0;
const int sampleWindow = 250; // Sample window width in mS (50 mS = 20Hz) 
unsigned int sample; 
double dBAverage=0;
float en=0; 
int count= 0,r=0;

int avance=0;

//Variables y objetos para el uso de ThingSpeak
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

//Objeto para el uso del PM25
HardwareSerial MySerial(1);//UART1


//Variables para almacenar los resultados
float number1 = 0;//PM2.5
float number2 = 0;//Intensidad del sonido
float number3 = 0;//Temperatura
float number4 = 0;//Humedad
float number5 = 0;//Gas Amonio

//Función usada para medir la intensidad del sonido
float intensidad_sonido(void);
//Función usada para medir la contaminación PM2.5
float leer_PM25(void);


void setup() {
   Serial.begin(115200);
   Serial.println("Sistema de medición de calidad del Aire");
   Serial.println("Comfamiliar Pereira");
   Serial.println("César Augusto Álvarez Gaspar");
   dht.begin();
   analogReadResolution(10);//ajuste resolucion esp32
   pinMode(AudioGain,OUTPUT);
   digitalWrite(AudioGain,HIGH); 
   display.init();
   display.setContrast(255);
   display.setTextAlignment(TEXT_ALIGN_LEFT);
   display.setFont(ArialMT_Plain_16);
   display.display();
   display.drawStringMaxWidth(0, 0,128, Bienvenida);
   display.display();
   WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
   //  MQ135 inicializacion

  Serial.println("Calibrando... \n");
  mQ135Ro = MQ135_nh4.MQCalibracion();
  Serial.print("Calibracion completa \n \n");
  Serial.print("MQ135 Ro=");
  Serial.print(mQ135Ro);
  Serial.println("kohm");

  delay(2000);

  if (!sensorPMA.begin()) {
    Serial.println("Falla en inicializacion HM330X");
    MySerial.begin(9600,SERIAL_8N1,pinRxPMB,pinTxPMB); 
    MySerial.setTimeout(1500);
    TypePM =TypePM_B; //Define el tipo de sensor que se usara  
     }else{
           Serial.println("HM330X inicializado");
     }
 
}

// the loop function runs over and over again forever
void loop() {
 number1 =leer_PM25();
 number2=intensidad_sonido();
 number3=dht.readTemperature();
 number4=dht.readHumidity();
 number5 = MQ135_nh4.MQGetGasPercentage(mQ135Ro); // Lectura Amoniaco
 Consola="";
 Consola+=number1;
 Consola+=" ";
 Consola+=number2;
 Consola+=" ";
 Consola+=number3;  
 Consola+=" ";
 Consola+=number4;
 Consola+=" ";
 Consola+=number5;  
 display.clear();
 display.drawStringMaxWidth(0, 0,128, Consola);
 display.display();
 // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
  }
    Serial.println("\nConnected.");
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);
  ThingSpeak.setField(3, number3);
  ThingSpeak.setField(4, number4);
  ThingSpeak.setField(5, number5);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
    Consola+=" ";
    Consola+="Canal Actualizado";
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
    Consola+=" ";
    Consola+="Error en Canal, Cod: ";
    Consola+=x;
  }
  Serial.println(Consola);
 
  delay(30000);

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


float leer_PM25(void)
{
float pm=0;
switch(TypePM)
{
 case TypePM_A:
               if (!sensorPMA.readSensor())
                   {
                    Serial.println("Falla en sensor PMA");
                   } else {
                           pm= sensorPMA.std.getPM2_5();//Función de lectura PM2.5
                          }
 break;
 case TypePM_B:
        if(MySerial.find(0x42)){    //start to read when detect 0x42
        delay(100);
        MySerial.readBytes(buf,LENG);
      if(buf[0] == 0x4d)
      {
        if(checkValue(buf,LENG))
          {
            pm=transmitPM2_5(buf);//count PM2.5 value of the air detector module
            //Conversión de ug/m3 a PM
            pm=(pm)/((28.01)*(1000/24.5));//Se compara con el CO2
          }
    }
  }
 break;
 default:
      Serial.println("Sensor PM 2.5 no especificado");
      pm=avance++;
      if(avance>1000)avance=0;
  break;
  
};
return pm;
}

char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;

  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
}
