/*  Capacitance Measurement
* Theory   A capacitor will charge, through a resistor, in one time constant, defined as T seconds where
 *    TC = R * C
 *    TC = time constant period in seconds
 *    R = resistance in ohms
 *    C = capacitance in farads (1 microfarad (ufd) = .0000001 farad = 10^-6 farads) 
 *    The capacitor's voltage at one time constant is defined as 63.2% of the charging voltage.    
* Humidity measurement from capacitance
 *    H = (Cc - Cs)/S + 55
 *    H = current Humidity (%)
 *    Cc = measured capacitance (pF)
 *    Cs = capacitance at 55% (pF)
 *    S = Sensitivity (RH)
 *    
*/

#include <OneWire.h>
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary)

#define analogPin      0          // analog pin for measuring capacitor voltage
#define chargePin      13         // pin to charge the capacitor - connected to one end of the charging resistor
#define dischargePin   11         // pin to discharge the capacitor
#define resistorValue  1000000.0F   // 1M change this to whatever resistor value you are using
                                  // F formatter tells compiler it's a floating point value
unsigned long startTime;
unsigned long elapsedTime;

float cc;                         // Measured Capacitance
float cs = 330.0;                 // Capacitance of HS07 at 55% 
float S = 0.6;                    // Sensitivity of HS07 (pF/%RH)
float RH;                         // Real Humidity

byte i;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
float celsius, fahrenheit;

void setup(){
  pinMode(chargePin, OUTPUT);     // set chargePin to output
  digitalWrite(chargePin, LOW);  

  Serial.begin(9600);             // initialize serial transmission for debugging
}

void loop(){

  /* Measure humidity from HS07 */
  digitalWrite(chargePin, HIGH);  // set chargePin HIGH and capacitor charging
  startTime = micros();
  
  while(analogRead(analogPin) < 647){    // 647 is 63.2% of 1023, which corresponds to full-scale voltage 
  }
  /* calculate capacitance */
  elapsedTime= micros() - startTime;
  // convert microseconds to seconds ( 10^-6 ) and Farads to picoFarads ( 10^12 ) net value 10^6
  cc = ((float)elapsedTime / resistorValue) * 1000000.0;         

  /* calculate humidity */
  RH = (cc - cs)/S/100.0 + 55.0;
  
  /* dicharge the capacitor  */
  digitalWrite(chargePin, LOW);             // set charge pin to  LOW 
  pinMode(dischargePin, OUTPUT);            // set discharge pin to output 
  digitalWrite(dischargePin, LOW);          // set discharge pin LOW 
  while(analogRead(analogPin) > 0){         // wait until capacitor is completely discharged
  }

  pinMode(dischargePin, INPUT);            // set discharge pin back to input
    if ( !ds.search(addr)) {
    // no more address found
    ds.reset_search();
    delay(250);
    return;
  }

  /* Measure Temperature from DS18S20 */
  if (OneWire::crc8(addr, 7) != addr[7]) {
      //CRC is not valid!
      return;
  }
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      // Chip = DS18S20 or old DS1820
      type_s = 1;
      break;
    case 0x28:
      // Chip = DS18B20
      type_s = 0;
      break;
    case 0x22:
      // Chip = DS1822
      type_s = 0;
      break;
    default:
      // Device is not a DS18x20 family device
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  // fahrenheit = celsius * 1.8 + 32.0;

  // print converted temperature and humidity readings
  Serial.print(celsius);
  Serial.print(",");
  Serial.println(RH);
}
