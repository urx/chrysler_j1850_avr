//*************************************************************************
//  Chrysler/Jeep CD changer emulator for J1850 bus
//  by Michael Wolf
//
//  Released under GNU GENERAL PUBLIC LICENSE
//
//  contact: webmaster@mictronics.de
//  homepage: www.mictronics.de
//
//  Revision History
//
//  when         what  who			why
//	02/09/06		 v1.00 Michael	Initial release
//
//*************************************************************************
#ifndef __MAIN_H__
#define __MAIN_H__

// define bit macros
#define SETBIT(x,y) (x |= (y)) 		// Set bit y in byte x
#define CLEARBIT(x,y) (x &= (~y)) // Clear bit y in byte x
#define CHECKBIT(x,y) (x & (y)) 	// Check bit y in byte x

#endif // __MAIN_H__
