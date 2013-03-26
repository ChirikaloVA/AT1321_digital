//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2003-2010 Daniel Vik
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef CRC32_H
#define CRC32_H

typedef unsigned char UInt8;
typedef unsigned long UInt32;

extern const UInt32 crc32Table[256];

// Calculate CRC tables based on a polynomial. (Default values for the
// tables are provided in the implementation. They are based on the 
// polynomial 0x04c11db7, used in e.g. PKZip, WinZip and Ethernet.
//void CalculateTables(UInt32 polynomial = 0x04c11db7);


inline unsigned long UpdateCRC32( char val, unsigned long crc );
unsigned long crc32( char *ptr, int size, unsigned long crc );
char* CRC32ROM(char* pStrCrc);


#endif /*CRC_H*/
