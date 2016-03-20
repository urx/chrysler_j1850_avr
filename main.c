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
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "main.h"
#include "j1850.h"


int16_t main( void )
{
  uint8_t j1850_rxmsg_buf[12];  // J1850 message buffer
  uint8_t j1850_txmsg_buf[] = {0x8D, 0x93, 0x01, 0x00, 0x00, 0x00};
  int8_t recv_nbytes;  // byte counter		

  j1850_init();	// init J1850 bus

  sei();	// enable global interrupts

  for(;;)
  {
    recv_nbytes = j1850_recv_msg(j1850_rxmsg_buf);	// get J1850 frame

    if( !(recv_nbytes & 0x80) ) // proceed only with no errors
    {
      if( j1850_rxmsg_buf[0] == 0x8D ){
	if( j1850_rxmsg_buf[1] == 0x0F ){
	  switch( j1850_rxmsg_buf[2] ){

	    default:
	      j1850_txmsg_buf[1] = 0x93;
	      j1850_txmsg_buf[2] = 0x01;
	      j1850_txmsg_buf[3] = 0x01;
	      j1850_txmsg_buf[4] = 0x80;
	      j1850_txmsg_buf[5] = j1850_crc( j1850_txmsg_buf,5);
	      j1850_send_msg(j1850_txmsg_buf, 6);
	      break;

	    case 0x21:
	      j1850_txmsg_buf[1] = 0x92;
	      j1850_txmsg_buf[2] = 0xC0;
	      j1850_txmsg_buf[3] = 0x00;
	      j1850_txmsg_buf[4] = 0x00;
	      j1850_txmsg_buf[5] = j1850_crc( j1850_txmsg_buf,5);
	      j1850_send_msg(j1850_txmsg_buf, 6);

	      j1850_txmsg_buf[1] = 0x92;
	      j1850_txmsg_buf[2] = 0xE1;
	      j1850_txmsg_buf[3] = 0x01;
	      j1850_txmsg_buf[4] = 0x03;
	      j1850_txmsg_buf[5] = j1850_crc( j1850_txmsg_buf,5);
	      j1850_send_msg(j1850_txmsg_buf, 6);

	      j1850_txmsg_buf[1] = 0x93;
	      j1850_txmsg_buf[2] = 0x01;
	      j1850_txmsg_buf[3] = 0x01;
	      j1850_txmsg_buf[4] = 0x80;
	      j1850_txmsg_buf[5] = j1850_crc( j1850_txmsg_buf,5);
	      j1850_send_msg(j1850_txmsg_buf, 6);
	      break;

	    case 0x24:
	      j1850_txmsg_buf[1] = 0x94;
	      j1850_txmsg_buf[2] = 0x00;
	      j1850_txmsg_buf[3] = 0x00;
	      j1850_txmsg_buf[4] = j1850_crc( j1850_txmsg_buf,4);
	      j1850_send_msg(j1850_txmsg_buf, 5);
	      break;

	  }
	}        
      }
    } // end if message recv
  }	// endless loop

  return 0;
} // end of main()
