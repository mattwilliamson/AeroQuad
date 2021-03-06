/*
  AeroQuad v3.0 - Nov 2011
 www.AeroQuad.com
 Copyright (c) 2011 Ted Carancho.  All rights reserved.
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

#ifndef _AQ_OSD_MAX7456_HEADING_H_
#define _AQ_OSD_MAX7456_HEADING_H_

//////////////////////////////////////////////////////////////////////////////
/////////////////////////// HeadingMagHold Display ///////////////////////////
//////////////////////////////////////////////////////////////////////////////

int lastHeading = 361; // bogus to force update

void displayHeading(float currentHeading) {
  int currentHeadingDeg = ((int)(currentHeading / M_PI * 180.0) + 360) % 360;
  if (currentHeadingDeg != lastHeading) {
    char buf[6];
    snprintf(buf,6,"\6%3d\7",currentHeadingDeg); // \6 is compass \7 is degree symbol
    writeChars( buf, 5, 0, COMPASS_ROW, COMPASS_COL );
    lastHeading = currentHeadingDeg;
  }
}

#endif  // #define _AQ_OSD_MAX7456_HEADING_H_


