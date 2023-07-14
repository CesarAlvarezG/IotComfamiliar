
#ifndef MQ135Sensor_h
#define MQ135Sensor_h
#include "arduino.h"

class mq135sensor
{
  private:

  int mq_pin;
  int bandera_gas;
  float rl;
  float r0_limpio;
  int muestras_calibracion;
  int espera_calibracion;
  int muestras_lectura;
  int espera_lectura;
  float NH4Curve[3] = {1,0.43,-0.28};  // {log(x),log(y),m} donde x e y son un punto de la gráfica
  float rs;

  public:

  mq135sensor(int pin, int gas);
  float MQResistenciaCalculada(int raw_adc);
  float MQCalibracion();
  float MQRead();
  float MQGetGasPercentage(float r0);
  float MQGetPercentage(float rs_ro_ratio, float *pcurve);
  
};
#endif
