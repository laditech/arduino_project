#ifndef gassensors_H
#define gassensors_H
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

/*MQ-GAS SENSORS LIBRARY*/
/************************Hardware Related Macros************************************/
#define         MQ2                 2
#define         MQ7                 7
#define         MQ135               135

#define         RL_VALUE_MQ2                  1           //define the load resistance connected to the gas sensor on the board, in kilo ohms
#define         RO_CLEAN_AIR_FACTOR_MQ2       9.577       //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO, which is extracted from the graph in datasheet

#define         RL_VALUE_MQ7                  1      
#define         RO_CLEAN_AIR_FACTOR_MQ7       26.09

#define         RL_VALUE_MQ135                1       
#define         RO_CLEAN_AIR_FACTOR_MQ135     3.59


/***********************Software Related Macros************************************/

#define         READ_SAMPLE_INTERVAL         (150)    //define how many samples you are going to take in normal operation
#define         READ_SAMPLE_TIMES            (15)     //define the time interal(in milisecond) between each samples in normal operation
                                                     
/**********************Application Related Macros**********************************/

#define         GAS_HYDROGEN                  (0)
#define         GAS_LPG                       (1)
#define         GAS_METHANE                   (2)
#define         GAS_CARBON_MONOXIDE           (3)
#define         GAS_ALCOHOL                   (4)
#define         GAS_SMOKE                     (5)
#define         GAS_PROPANE                   (6)
#define         GAS_BENZENE                   (7)
#define         GAS_HEXANE                    (8)
#define         GAS_CARBON_DIOXIDE            (9)
#define         GAS_AMMONIUM                  (10)
#define         GAS_TOLUENE                   (11)
#define         GAS_ACETONE                   (12)

//#define         accuracy                      (0)   //for linearcurves
#define         accuracy                    (1)   //for nonlinearcurves, un comment this line and comment the above line if calculations
                                                    //are to be done using non linear curve equations
                                                    
/*****************************Gassensors class***********************************************/

class Gassensors {
  public:
    Gassensors(uint8_t pin, uint8_t type);
    float MQResistanceCalculation(int raw_adc);
    float MQRead(int mq_pin);
    int MQGetGasPercentage(float rs_ro_ratio, int gas_id);
    int gasppm(uint8_t pin, uint8_t type,int mqgases_id );
    float rs_ro_ratio(uint8_t pin);
    void printallgases(uint8_t type);
  private:
    uint8_t _pin, _type, RL_VALUE=1, RO_CLEAN_AIR_FACTOR;
    float _Rovalues[3]={8.82, 1.9, 17.85}; 
    // Run the Gassensors_calibration.ino file in clean air conditions to find out Rovalues and enter them in this array                    
};
#endif

Gassensors::Gassensors(uint8_t pin, uint8_t type) {
  _pin = pin;
  _type = type;
}

  /****************** MQResistanceCalculation ****************************************
  Input:   raw_adc - raw value read from adc, which represents the voltage
  Output:  the calculated sensor resistance
  Remarks: The sensor and the load resistor forms a voltage divider. Given the voltage
         across the load resistor and its resistance, the resistance of the sensor
         could be derived.
************************************************************************************/
float Gassensors::MQResistanceCalculation(int raw_adc)
{
  /*Serial.print(F("printing raw adc in mqresistancecalc: "));
  Serial.println(raw_adc);
  Serial.print(F("printing RL_value in mqresistancecalc: "));
  Serial.println(RL_VALUE);
  Serial.print(F("printing Rs value in mqresistancecalc: "));
  Serial.println(((float)RL_VALUE * (1023 - raw_adc) / raw_adc));*/
  return ( ((float)RL_VALUE * (1023 - raw_adc) / raw_adc));
}
/*****************************  MQRead *********************************************
  Input:   mq_pin - analog channel
  Output:  Rs of the sensor
  Remarks: This function use MQResistanceCalculation to caculate the sensor resistenc (Rs).
         The Rs changes as the sensor is in the different consentration of the target
         gas. The sample times and the time interval between samples could be configured
         by changing the definition of the macros.
************************************************************************************/
float Gassensors::MQRead(int mq_pin)
{
  int i;
  float rs = 0;

  for (i = 0; i < READ_SAMPLE_TIMES; i++) {
   // Serial.print(F("printing raw adc in mqread: "));
   // Serial.println(analogRead(mq_pin));
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  //  Serial.print(F("printing rs value in mqread: "));
   // Serial.println(rs);    
  }
 
  rs = rs / READ_SAMPLE_TIMES;
  //Serial.print(F("rs value in mq read:  "));
 // Serial.println(rs);
  return rs;
}

/*****************************  MQGetGasPercentage **********************************
  Input:   rs_ro_ratio - Rs divided by Ro
         gas_id      - target gas type
  Output:  ppm of the target gas
  Remarks: This function uses different equations representing curves of each gas to
         calculate the ppm (parts per million) of the target gas(or mg/L (milligrams
         per litre) in case of MQ3 sensor.
************************************************************************************/
int Gassensors::MQGetGasPercentage(float rs_ro_ratio, int gas_id )
{
  switch (_type) {

    case MQ2:
    //Serial.println("calculating mq2 co gases"); 
              // Serial.print("rs_ro_ratio:  ");                                                        //debug statements that display step by step calculation of ppm
              // Serial.println(rs_ro_ratio);
              /* Serial.print("log of rs_ro_ratio: ");
               Serial.println(log10(rs_ro_ratio));
               Serial.print("3.891* log of rs_ro_ratio: ");
               Serial.println((-3.891 * (log10(rs_ro_ratio))));
               Serial.print("pow of 10, 3.891* log of rs_ro_ratio: ");
               Serial.println(pow(10, ((-3.891 * (log10(rs_ro_ratio))))));
               Serial.print("2.75+pow of 10, 3.891* log of rs_ro_ratio: ");
               Serial.println((pow(10, ((-3.891 * (log10(rs_ro_ratio))) + 2.750))));*/
     // Serial.print(F("calculating mq2 gases: xxxxxx     "));
      //Serial.print(gas_id);
         //   Serial.print(F("gas_id      "));

        if ( accuracy == 1 ) {
        if ( gas_id == GAS_SMOKE && rs_ro_ratio<=3.4 && rs_ro_ratio>=0.60) {
          return (pow(10, (-0.976 * pow((log10(rs_ro_ratio)), 2) - 2.018 * (log10(rs_ro_ratio)) + 3.617)));
        } 
      }
      break;

    case MQ7:
      //Serial.print(F("calculating mq7 gases"));
      if ( accuracy == 1 ) {
        if ( gas_id == GAS_CARBON_MONOXIDE && rs_ro_ratio<= 1.59 && rs_ro_ratio>= 0.09) {
          return (pow(10, ((-1.525 * (log10(rs_ro_ratio))) + 1.994)));
        } 
      }
      break;

    case MQ135:
      //Serial.print(F("calculating mq135 gases: xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx     "));
     // Serial.print(rs_ro_ratio);
    //  //Serial.print(gas_id);
        //          //Serial.print("gas_id      ");
               Serial.print("rs_ro_ratio:  ");                                                        //debug statements that display step by step calculation of ppm
               Serial.println(rs_ro_ratio);
        if ( accuracy == 1 ) {
         /*Serial.println("calculating mq135 co gases"); 
               Serial.print("rs_ro_ratio:  ");                                                        //debug statements that display step by step calculation of ppm
               Serial.println(rs_ro_ratio);
               Serial.print("log of rs_ro_ratio: ");
               Serial.println(log10(rs_ro_ratio));
               Serial.print("3.891* log of rs_ro_ratio: ");
               Serial.println((-3.891 * (log10(rs_ro_ratio))));
               Serial.print("pow of 10, 3.891* log of rs_ro_ratio: ");
               Serial.println(pow(10, ((-3.891 * (log10(rs_ro_ratio))))));
               Serial.print("2.75+pow of 10, 3.891* log of rs_ro_ratio: ");
               Serial.println((pow(10, ((-3.891 * (log10(rs_ro_ratio))) + 2.750))));*/
        if ( gas_id == GAS_CARBON_DIOXIDE && rs_ro_ratio<=2.3 && rs_ro_ratio>=0.3) {
          return (pow(10, ((-2.890 * (log10(rs_ro_ratio))) + 2.055)));
        } 
      }
      break;
  }
  return 0;
}

float Gassensors::rs_ro_ratio(uint8_t pin)
{
  switch (_type) {
    case MQ2:
      /*{Serial.print("   rs2 value now: ");
      float x = MQRead(pin);
      Serial.print(x);
      Serial.print("    r02 value now:  ");
      Serial.print(_Ro2);
       Serial.print("   rs_ro2_ratio now: ");
      Serial.print(x/_Ro2);
      return x/_Ro2;}
      //Serial.print(MQRead(pin) / _Ro2);*/
      return MQRead(34) / _Rovalues[0];
      break;

    case MQ7:
      return MQRead(36) / _Rovalues[5];
      break;

    case MQ135:
      //Serial.println(F("rs_ro135_ratio"));
       //Serial.println(MQRead(pin));
     // Serial.println(MQRead(pin) / _Rovalues[8]);
      return MQRead(35)/_Rovalues[8];
      break;

    default:
      Serial.print(F("rs_ro default case executed"));

  }
}
  int Gassensors::gasppm(uint8_t pin, uint8_t type, int mqgases_id)
  {
    return (MQGetGasPercentage(rs_ro_ratio(pin), mqgases_id ));
  }

  void Gassensors::printallgases(uint8_t type)
  {//Serial.println(_Rovalues[_type-2]);
    switch (_type) {
        case MQ2:
        {int mq2gases[]={0,1,2,4,5,6};
        Serial.println(F("MQSENSOR 2 output"));
        Serial.print(F("SMOKE:"));
        Serial.print(gasppm(_pin, _type, mq2gases[5]));
        Serial.print( F(" PPM") );
        Serial.print(F("    "));
        Serial.print(F("\n"));}
        break;

      case MQ7:
        {int mq7gases[] = {3};
        Serial.println(F("MQSENSOR 7 output"));
        Serial.print(F("CARBON_MONOXIDE:"));
        Serial.print(gasppm(_pin, _type, mq7gases[0]));
        Serial.print(F( " PPM" ));
        Serial.print(F("\n"));}
        break;

      case MQ135:
        {int mq135gases[] = {7,9,10};
        Serial.println(F("MQSENSOR 135 output"));
        Serial.print(F("CARBON_DIOXIDE:"));
        Serial.print(gasppm(_pin, _type, mq135gases[0]));
        Serial.print(F( " PPM" ));
        Serial.print(F("    "));
        Serial.print(F("\n"));}
        break;
    }
  }
