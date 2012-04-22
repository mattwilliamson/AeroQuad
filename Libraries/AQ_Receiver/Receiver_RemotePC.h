/*
  AeroQuad v3.0.1 - February 2012
  www.AeroQuad.com
  Copyright (c) 2012 Ted Carancho.  All rights reserved.
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

#ifndef _AEROQUAD_RECEIVER_REMOTE_PC_H_
#define _AEROQUAD_RECEIVER_REMOTE_PC_H_

#include "Arduino.h"
#include "Receiver.h"

#define RX_PACKET_LENGTH 8

// Packets for 8 channels looks like this. 
  // 10 chars (0-250)
  // chr(254) terminated

// 'Q' | THROTTLE | PITCH | ROLL | YAW | MODE | AUX1 | AUX2 | AUX3 | 254
// ---------------------------------------------------------------------
//  Q  |   120    |   0   |   0  | 250 | 250  |  0   |  0   |  0   |  ?

// Declare globals for SerialCom.h
byte rxChannelMap[] = {THROTTLE, YAXIS, XAXIS, ZAXIS, MODE, AUX1, AUX2, AUX3};
byte rxBuffer[RX_PACKET_LENGTH];
byte rxBytesReceived;

void initializeReceiver(int nbChannel) {

  initializeReceiverParam(nbChannel);
  for (byte channel = XAXIS; channel < THROTTLE; channel++) {
    receiverCommand[channel] = 1500;
    receiverZero[channel] = 1500;
  }
  receiverCommand[THROTTLE] = 0;
  receiverZero[THROTTLE] = 0;
  receiverCommand[MODE] = 2000;
  receiverZero[MODE] = 0;
  receiverCommand[AUX1] = 2000;
  receiverZero[AUX1] = 0;
}

int getRawChannelValue(byte channel) {
  return receiverCommand[channel];
}
  
void setChannelValue(byte channel,int value) {
  receiverCommand[channel] = value;
}

void readReceiverPC() {
  rxBytesReceived = 0;
  
  while (rxBytesReceived < RX_PACKET_LENGTH && SERIAL_AVAILABLE()) {
    rxBuffer[rxBytesReceived] = SERIAL_READ();
    rxBytesReceived++;
  }
  
  // Only accept the packet if it's long enough and is terminated with char(254)
  if (SERIAL_AVAILABLE()) {
    int lastChar = SERIAL_READ();

    if (lastChar == 254 && rxBytesReceived >= RX_PACKET_LENGTH) {
      for (int i=0; i < RX_PACKET_LENGTH; i++) {
        // We are packing ints up to 1000 into one byte, so we divide by four
        setChannelValue(rxChannelMap[i], rxBuffer[i] * 4 + 1000);
      }
    }
  }
}


#endif



