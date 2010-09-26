/*
  AeroQuad v2.1 - September 2010
  www.AeroQuad.com
  Copyright (c) 2010 Ted Carancho.  All rights reserved.
  An Open Source Arduino based multicopter.
 
  This program is free software: you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version. 

  This program is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details. 

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>. 
*/

// Class to define sensors that can determine altitude

// ***********************************************************************
// ************************** Altitude Class *****************************
// ***********************************************************************

class Altitude {
public:
  float altitude, rawAltitude;
  float groundTemperature; // remove later
  float groundPressure; // remove later
  float groundAltitude;
  float smoothFactor;
  
  Altitude (void) { 
    altitude = 0;
    smoothFactor = 0.1;
  }

  // **********************************************************************
  // The following function calls must be defined inside any new subclasses
  // **********************************************************************
  virtual void initialize(void); 
  virtual void measure(void);
  
  // *********************************************************
  // The following functions are common between all subclasses
  // *********************************************************
  const float getData(void) {
    return altitude;
  }
  
  const float getRawData(void) {
    return rawAltitude;
  }
  
  void measureGround(void) {
    // measure initial ground pressure (multiple samples)
    for (int i=0; i < 20; i++) {
      measure();
      delay(26);
      groundAltitude = smooth(altitude, groundAltitude, 0.5);
      //Serial.println(groundAltitude);
    }
  }
  
  void setGroundAltitude(float value) {
    groundAltitude = value;
  }
  
  const float getGroundAltitude(void) {
    return groundAltitude;
  }
  
  void setSmoothFactor(float value) {
    smoothFactor = value;
  }
};

// ***********************************************************************
// ************************* BMP085 Subclass *****************************
// ***********************************************************************
class Altitude_AeroQuad_v2 : public Altitude {
// This sets up the BMP085 from Sparkfun
// Code from http://wiring.org.co/learning/libraries/bmp085.html
// Also made bug fixes based on BMP085 library from Jordi Munoz and Jose Julio
private:
  byte overSamplingSetting;
  int ac1, ac2, ac3;
  unsigned int ac4, ac5, ac6;
  int b1, b2, mb, mc, md;
  long pressure;
  int temperature;
  int altitudeAddress;
  long rawPressure, rawTemperature;
  byte select, pressureCount;
  float pressureFactor;
  
  void requestRawPressure(void) {
    updateRegisterI2C(altitudeAddress, 0xF4, 0x34+(overSamplingSetting<<6));
  }
  
  long readRawPressure(void) {
    unsigned char msb, lsb, xlsb;
    sendByteI2C(altitudeAddress, 0xF6);
    Wire.requestFrom(altitudeAddress, 3); // request three bytes
    return (((long)Wire.receive()<<16) | ((long)Wire.receive()<<8) | ((long)Wire.receive())) >>(8-overSamplingSetting);
  }

  void requestRawTemperature(void) {
    updateRegisterI2C(altitudeAddress, 0xF4, 0x2E);
  }
  
  long readRawTemperature(void) {
    sendByteI2C(altitudeAddress, 0xF6);
    return readWordI2C(altitudeAddress);
  }

public: 
  Altitude_AeroQuad_v2() : Altitude(){
    altitudeAddress = 0x77;
    // oversampling setting
    // 0 = ultra low power
    // 1 = standard
    // 2 = high
    // 3 = ultra high resolution
    overSamplingSetting = 3;
    pressure = 0;
    groundPressure = 0;
    temperature = 0;
    groundTemperature = 0;
    groundAltitude = 0;
    pressureFactor = 1/5.255;
  }

  // ***********************************************************
  // Define all the virtual functions declared in the main class
  // ***********************************************************
  void initialize(void) {
    int buffer[22];
    
    sendByteI2C(altitudeAddress, 0xAA); // Read calibration data registers
    Wire.requestFrom(altitudeAddress, 22);
    for (int i=0; i<22; i++) {
      while(!Wire.available()); // wait until data available
      buffer[i] = Wire.receive();
    }    
    ac1 = (buffer[0] << 8) | buffer[1];
    ac2 = (buffer[2] << 8) | buffer[3];
    ac3 = (buffer[4] << 8) | buffer[5];
    ac4 = (buffer[6] << 8) | buffer[7];
    ac5 = (buffer[8] << 8) | buffer[9];
    ac6 = (buffer[10] << 8) | buffer[11];
    b1 = (buffer[12] << 8) | buffer[13];
    b2 = (buffer[14] << 8) | buffer[15];
    mb = (buffer[16] << 8) | buffer[17];
    mc = (buffer[18] << 8) | buffer[19];
    md = (buffer[20] << 8) | buffer[21];
    Wire.endTransmission();
    requestRawTemperature(); // setup up next measure() for temperature
    select = TEMPERATURE;
    pressureCount = 0;
    delay(5);
  }
  
  void measure(void) {
    long x1, x2, x3, b3, b5, b6, p, tmp;
    unsigned long b4, b7;

    // switch between pressure and tempature measurements
    // each loop, since it's slow to measure pressure
    if (select == PRESSURE) {
      rawPressure = readRawPressure();
      if (pressureCount == 3) {
        requestRawTemperature();
        pressureCount = 0;
       select = TEMPERATURE;
      }
      else
        requestRawPressure();
      pressureCount++;
    }
    else { // select must equal TEMPERATURE
      rawTemperature = readRawTemperature();
      requestRawPressure();
      select = PRESSURE;
    }
    
    //calculate true temperature
    x1 = ((long)rawTemperature - ac6) * ac5 >> 15; // rawTemperature from requestRawTemperature();
    x2 = ((long) mc << 11) / (x1 + md);
    b5 = x1 + x2;
    temperature = (b5 + 8) >> 4;
  
    //calculate true pressure
    b6 = b5 - 4000;
    x1 = (b2 * (b6 * b6 >> 12)) >> 11; 
    x2 = ac2 * b6 >> 11;
    x3 = x1 + x2;
    b3 = (((ac1*4 + x3)<<overSamplingSetting)+2)/4;
    x1 = ac3 * b6 >> 13;
    x2 = (b1 * (b6 * b6 >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (ac4 * (uint32_t) (x3 + 32768)) >> 15;
    b7 = ((uint32_t) rawPressure - b3) * (50000 >> overSamplingSetting); // rawPressure from requestRawPressure();
    p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;
    
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    pressure = (p + ((x1 + x2 + 3791) >> 4));
    
    rawAltitude = 44330 * (1 - pow(pressure/101325.0, pressureFactor)); // returns absolute altitude in meters
    altitude = smooth(rawAltitude, altitude, smoothFactor); // smoothFactor defined in main class
  }
};