#include "TimerOne.h"               // include TimerOne.h
#include "FastLED.h"              //include fast led libary for timer (ironically)

//pin assignments:
const byte redLED = 2;
const byte greenLED = 3;
const byte blueLED = 4;
const byte ntcPin = A0;                      //pin connected to NTC
const byte feedBackPIN  = A1;               //pin connected to voltage feedback of SG3525
const byte shuntPin = A2;                 //pin connected to shunt resistors
const byte batteryVoltagePIN = A3;          //pin connected to battery voltage devider
const byte sg3525EN = 5;                    //pull low to enable the sg3525
const byte tapIndicatorPIN = 7;            //pin to indicator which tap the transformer is tuned to. High is higher tap turns
const byte fanPin = 6;                       //pin connected to the fan

//initialization values
int period = 16667;        //period of the wave, 16667 for 60Hz, 20000 for 50Hz (in microseconds)
int dutycycle = 0;               // Initailize duty cylce variable as integer data type
int sense_value = 0;               // Initialize sense_value variable to capture the adc reading of battery voltage (range from 0 to 1023)
float battery_voltage = 0.0;               // Initialize battery_voltage variable as a float data type to convert sense_value to actual battery voltage
float currentValue;      //current value 
float avgCurrent;      //averaged current value
float R1 = 1000;        //resistor connected in series with NTC
float logR2, R2, T;      //ntc values
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07; //ntc 3950 constants
float avgTempADC;
float tempValues;
byte avgTimes;
byte avgTimes1;
int currentValueRAW;
 
//protection mechnisms & hardware config
float overVoltageLIMIT;       //overvoltage limit value
float underVoltageLIMIT;       //undervoltage limit value
float voltageFB;              //sg3525 feedback voltage
float packageTemp;               //thermister measured temp value
float batteryVoltage;               // Initialize battery_voltage 
float overCurrentLIMIT = 15; //x numbers of Amps to trigger over current proection
const float lowTapMIN_voltage = 14.4;  //min voltage for low tap mode
const float lowTapMAX_voltage = 21.1;  //max voltage for low tap mode
const float highTapMIN_voltage = 11.1;  //min voltage for low tap mode
const float highTapMAX_voltage = 14.5;  //max voltage for low tap mode
const float maxTempLIMIT = 80;   //max temperature allowed in C
const float fanOnTEMP = 44;     //fan turn on temp
const int vDivUP = 10; //battery voltage divider upper resistance in K ohms
const long vDivLOW = 2;  //battery voltage divider lower resistance in K ohms
const float shuntValue = 0.0125; //shunt resistor value in ohm
const byte currentReadTIMES = 15; //times to read current value before averaging

//states or modes: 
bool highTapINDICATE; //if true, higher secondary winding is selected
bool inverterOn = true;  //if true, turn the inverter stage on
bool halfPowerMODE = false; //set the outpower to half if true, used to drive pure resistive loads that requre too much power.
bool overVoltagePROTECTION = false;  //initialze these values in false state
bool underVoltagePROTECTION = false;
bool overCurrentPROTECTION = false;
bool overTempPROTECTION = false;
bool stateChange = false;

                                                 
void setup() {
  delay(500);
  Serial.begin(9600);
  pinMode (tapIndicatorPIN, INPUT); //set tapindicator pin as input
  pinMode (redLED,OUTPUT);
  pinMode (greenLED,OUTPUT);
  pinMode (blueLED,OUTPUT);
  pinMode (fanPin,OUTPUT);
  pinMode (9,OUTPUT);              // set pin 9 as an output pin for pwm (DO NOT CHANGE THIS PIN)
  pinMode (10,OUTPUT);             // set pin 10 as an output pin for pwm (DO NOT CHANGE THIS PIN)

  digitalWrite(redLED,HIGH);      // start up in red state
  
   senseValues;                     //check various values 
   printValues;                     //print initial values

  
  Timer1.initialize(16667);             // Initailize timer1 time period 16667 for 60HZ, 20000 for 50HZ
  TCCR1A = (TCCR1A & 0x0F) | 0xB0 ;             // set pin 10 inverted of pin 9
  dutycycle = 300;
  
}  //end void setup


 void loop() {             // loop function starts
  EVERY_N_MILLISECONDS(500){
    printValues (); 
  } //end every N milli sec actions

  EVERY_N_MILLISECONDS(20){
    senseValues (); 
  } //end every N milli sec actions

  if (
    overVoltagePROTECTION == true ||
    underVoltagePROTECTION == true ||
    overCurrentPROTECTION == true ||
    overTempPROTECTION == true ||
    stateChange == true ) {
      digitalWrite(sg3525EN, LOW); 
      inverterOn = false;
      digitalWrite(greenLED, LOW ); 
      digitalWrite(redLED, HIGH);   
    } //end turn off inverter codes
    else { inverterOn = true; } //if nothing is wrong and not changing state, turn the inverter on
    
    
    
  if( inverterOn == true ) {
  digitalWrite(redLED, LOW);  
  //Serial.println(currentValue);
  digitalWrite(greenLED, HIGH);  
  digitalWrite(sg3525EN, HIGH);  
  
  if (batteryVoltage <= 11.5){ dutycycle = 300; }
  else if ( halfPowerMODE == true ) { dutycycle = 135;}
  else{ dutycycle = 270; } 
  
  Timer1.pwm(9.,dutycycle,period);              // Timer1.pwm function takes argument as (pin no. , dutycycle , time period)
  Timer1.pwm(10,1023-dutycycle,period);   
  } //end if inverter is on functions

  if(inverterOn == false ) {
    dutycycle = 0; //turn H-bridge off.
  } //end inverter off
                                                                   
}               // loop function ends

void senseValues() {                      //senser values function starts

  int feedbackVoltageRAW = analogRead(feedBackPIN); 
  voltageFB =  feedbackVoltageRAW * (5.0 / 1023.0);    //calculating the feedback voltage
  
  int voltageSenseRAW = analogRead(batteryVoltagePIN); //reading raw ADC battety voltage value
  batteryVoltage = voltageSenseRAW * (5.0 / 1023.0) * ((vDivUP + vDivLOW) / vDivLOW); //calculating the input voltage

  currentValueRAW = analogRead(shuntPin);
    if (avgTimes1 <= currentReadTIMES) {
    currentValue = currentValue + currentValueRAW;
    avgTimes1++;
  } //end if 
  if(avgTimes1 > currentReadTIMES) {
     currentValue = currentValue / currentReadTIMES;
     avgCurrent = (currentValue * (5.0 / 1023.0)) / (shuntValue);
     currentValue = 0;
     avgTimes1 = 0;
  }//end calculating average current
  

  int tempSenceRAW = analogRead(ntcPin); 
  if (avgTimes <= 10) {
    tempValues = tempValues + tempSenceRAW;
    avgTimes++;
  } //end if 

  if (avgTimes > 10) {
    avgTempADC = tempValues / avgTimes;
    tempValues = 0;
    avgTimes = 0;
    R2 = R1 * (1023.0 / (float)avgTempADC - 1.0);
    logR2 = log(R2);
    packageTemp = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
    packageTemp = packageTemp - 273.15;
  } //end if

  highTapINDICATE = digitalRead(tapIndicatorPIN);
  if (highTapINDICATE == true) { //if high tap is indicated, use high tap limits
    overVoltageLIMIT = highTapMAX_voltage;
    underVoltageLIMIT = highTapMIN_voltage;
  } //end if high turn tap is selected

   if (highTapINDICATE == false) {
    overVoltageLIMIT = lowTapMAX_voltage;
    underVoltageLIMIT = lowTapMIN_voltage;
  } //end if low turn tap is selected


if ( batteryVoltage > overVoltageLIMIT ) {
  overVoltagePROTECTION = true;
  Serial.println("OVER VOLTAGE");
} // end if over voltage

if ( batteryVoltage < underVoltageLIMIT ) {
  //underVoltagePROTECTION = true;
  Serial.println("UNDER VOLTAGE");
} // end if under voltage

if (avgCurrent > overCurrentLIMIT) {
  overCurrentPROTECTION = true;
  Serial.println("OVER CURRENT");
  Serial.println(avgCurrent);
} //end if over current

if (packageTemp > maxTempLIMIT) {
  overTempPROTECTION = true; 
  Serial.println("OVER TEMP");
} //end if over Temp

if (packageTemp > fanOnTEMP) {
  digitalWrite(fanPin, HIGH);
} //end if fan on actions
else{
  digitalWrite(fanPin, LOW);
}
                           
}          //sense values function ends

void printValues () {
  Serial.print("battery voltage = ");
  Serial.println(batteryVoltage);
  Serial.print("Current = ");
  Serial.println(avgCurrent);
  Serial.println(currentValueRAW);
  Serial.println(currentValue);
  Serial.print("Temp = ");
  Serial.println(packageTemp);
  Serial.print("feedback Voltage = ");
  Serial.println(voltageFB);
  Serial.print("high tap selected = ");
  Serial.println(highTapINDICATE);
  Serial.print("Max input voltage = ");
  Serial.println(overVoltageLIMIT);
  Serial.println("_____________________");
} //end print values
                  
                                                 
