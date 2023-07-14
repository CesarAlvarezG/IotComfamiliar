
#include "MQ135Sensor.h"

mq135sensor::mq135sensor(int pin, int gas)
{
     mq_pin = pin;
     rl = 1;
     r0_limpio = 3.8;
     muestras_calibracion =50;
     espera_calibracion = 500;
     muestras_lectura = 5;
     espera_lectura = 50;
     bandera_gas = gas;
     
}

float mq135sensor::MQResistenciaCalculada(int raw_adc)
{
  float y = 0;
  y = (rl*(4096-raw_adc)/raw_adc);
  return (((float) y * (3/2)));
}

float mq135sensor::MQCalibracion()
{
  int i;
  float val = 0;
  float r0 = 0;

  for(i=0;i<muestras_calibracion;i++){
    r0 += MQResistenciaCalculada(analogRead(mq_pin));        
    delay(espera_calibracion);
  }

  r0 = r0/muestras_calibracion;
  r0 = r0/r0_limpio;
  return r0;
}

float mq135sensor::MQRead()
{
  int i;
  rs = 0;
  for(i=0;i<muestras_lectura;i++){
    rs += MQResistenciaCalculada(analogRead(mq_pin));
    delay(espera_lectura);
  }
  rs = rs/muestras_lectura;
  return rs;
}

float mq135sensor::MQGetGasPercentage(float r0)
{
  MQRead();
  if(bandera_gas == 0){
    return MQGetPercentage((rs/r0),NH4Curve);
 }
}   
 

float mq135sensor::MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return {pow(10,(((log(rs_ro_ratio)-pcurve[1])/pcurve[2])+pcurve[0]))};
}
