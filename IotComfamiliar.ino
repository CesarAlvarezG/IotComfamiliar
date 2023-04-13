#include <Wire.h>
#include "SSD1306Wire.h"
#include "DHT.h"

#define DTHPIN 18
#define DTHTYPE DHT11

DHT dht(DTHPIN, DTHTYPE);
SSD1306Wire display(0x3c, SDA, SCL);

const String Bienvenida="Sistema de medición de calidad del Aire";
String Consola="";
float h=0;//Humedad
float t=0;//Temperatura

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
}

// the loop function runs over and over again forever
void loop() {
 h=dht.readHumidity();
 t=dht.readTemperature();
 Serial.print(h);
 Serial.print(" ");
 Serial.println(t);
 Consola="";
 Consola+=h;
 Consola+=" ";
 Consola+=t;  
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
