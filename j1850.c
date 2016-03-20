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
#include <avr/io.h>
#include "j1850.h"
/* 
 **--------------------------------------------------------------------------- 
 ** 
 ** Abstract: Init J1850 bus driver
 ** 
 ** Parameters: none
 ** 
 ** Returns: none
 ** 
 **--------------------------------------------------------------------------- 
 */ 
void j1850_init(void)
{
  j1850_passive();	// set VPW pin in passive state
  J1850_DIR_OUT |= _BV(J1850_PIN_OUT);	// make VPW output pin an output

  J1850_PULLUP_IN |= _BV(J1850_PIN_IN);	// enable pull-up on VPW pin
  J1850_DIR_IN	&=~ _BV(J1850_PIN_IN);	// make VPW input pin an input
  timeout_multiplier = 0x19;	// set default timeout to 4ms * 25 = 100ms


}


/* 
 **--------------------------------------------------------------------------- 
 ** 
 ** Abstract: Wait for J1850 bus idle
 ** 
 ** Parameters: none
 ** 
 ** Returns: none
 ** 
 **--------------------------------------------------------------------------- 
 */ 
static void j1850_wait_idle(void)
{
  timer0_start();
  while(TCNT0 < RX_IFS_MIN)	// wait for minimum IFS symbol
  {
    if(is_j1850_active()) timer0_start();	// restart timer0 when bus not idle
  }
}


/* 
 **--------------------------------------------------------------------------- 
 ** 
 ** Abstract: Receive J1850 frame (max 12 bytes)
 ** 
 ** Parameters: Pointer to frame buffer
 ** 
 ** Returns: Number of received bytes OR in case of error, error code with
 **          bit 7 set as error indication
 **
 **--------------------------------------------------------------------------- 
 */ 
uint8_t j1850_recv_msg(uint8_t *msg_buf )
{
  uint8_t nbits;			// bit position counter within a byte
  uint8_t nbytes;		// number of received bytes
  uint8_t bit_state;// used to compare bit state, active or passive
  /*
     wait for responds
     */
  timer0_start();	
  while(!is_j1850_active())	// run as long bus is passive (IDLE)
  {
    if(TCNT0 >= WAIT_100us)	// check for 100us
    {
      timer0_stop();
      return J1850_RETURN_CODE_NO_DATA | 0x80;	// error, no responds within 100us
    }
  }

  // wait for SOF
  timer0_start();	// restart timer1
  while(is_j1850_active())	// run as long bus is active (SOF is an active symbol)
  {
    if(TCNT0 >=  RX_SOF_MAX) return J1850_RETURN_CODE_BUS_ERROR | 0x80;	// error on SOF timeout
  }

  timer0_stop();
  if(TCNT0 < RX_SOF_MIN) return J1850_RETURN_CODE_BUS_ERROR | 0x80;	// error, symbole was not SOF

  bit_state = is_j1850_active();	// store actual bus state
  timer0_start();
  for(nbytes = 0; nbytes < 12; ++nbytes)
  {
    nbits = 8;
    do
    {
      *msg_buf <<= 1;
      while(is_j1850_active() == bit_state) // compare last with actual bus state, wait for change
      {
	if(TCNT0 >= RX_EOD_MIN	)	// check for EOD symbol
	{
	  timer0_stop();
	  return nbytes;	// return number of received bytes
	}
      }
      bit_state = is_j1850_active();	// store actual bus state
      timer0_stop();
      if( TCNT0 < RX_SHORT_MIN) return J1850_RETURN_CODE_BUS_ERROR | 0x80;	// error, pulse was to short

      // check for short active pulse = "1" bit
      if( (TCNT0 < RX_SHORT_MAX) && !is_j1850_active() )
	*msg_buf |= 1;

      // check for long passive pulse = "1" bit
      if( (TCNT0 > RX_LONG_MIN) && (TCNT0 < RX_LONG_MAX) && is_j1850_active() )
	*msg_buf |= 1;

      timer0_start();	// restart timer
    } while(--nbits);// end 8 bit while loop

    ++msg_buf;	// store next byte

  }	// end 12 byte for loop

  // return after a maximum of 12 bytes
  timer0_stop();	
  return nbytes;
}


/* 
 **--------------------------------------------------------------------------- 
 ** 
 ** Abstract: Send J1850 frame (maximum 12 bytes)
 ** 
 ** Parameters: Pointer to frame buffer, frame length
 ** 
 ** Returns: 0 = error
 **          1 = OK
 ** 
 **--------------------------------------------------------------------------- 
 */ 
uint8_t j1850_send_msg(uint8_t *msg_buf, int8_t nbytes)
{
  if(nbytes > 12)	return J1850_RETURN_CODE_DATA_ERROR;	// error, message to long, see SAE J1850

  j1850_wait_idle();	// wait for idle bus

  timer0_start();	
  j1850_active();	// set bus active

  while(TCNT0 < TX_SOF);	// transmit SOF symbol

  uint8_t temp_byte,	// temporary byte store
	  nbits;		// bit position counter within a byte	

  uint16_t delay;		// bit delay time

  do
  {
    temp_byte = *msg_buf;	// store byte temporary
    nbits = 8;
    while (nbits--)		// send 8 bits
    {
      if(nbits & 1) // start allways with passive symbol
      {
	j1850_passive();	// set bus passive
	timer0_start();
	delay = (temp_byte & 0x80) ? TX_LONG : TX_SHORT;	// send correct pulse lenght
	while (TCNT0 <= delay)	// wait
	{
	  if(!J1850_PORT_IN & _BV(J1850_PIN_IN))	// check for bus error
	  {
	    timer0_stop();
	    return J1850_RETURN_CODE_BUS_ERROR;	// error, bus collision!
	  }
	}
      }
      else	// send active symbol
      {
	j1850_active();	// set bus active
	timer0_start();
	delay = (temp_byte & 0x80) ? TX_SHORT : TX_LONG;	// send correct pulse lenght
	while (TCNT0 <= delay);	// wait
	// no error check needed, ACTIVE dominates
      }
      temp_byte <<= 1;	// next bit
    }// end nbits while loop
    ++msg_buf;	// next byte from buffer
  } while(--nbytes);// end nbytes do loop

  j1850_passive();	// send EOF symbol
  timer0_start();
  while (TCNT0 <= TX_EOF); // wait for EOF complete
  timer0_stop();
  return J1850_RETURN_CODE_OK;	// no error
}

/* 
 **--------------------------------------------------------------------------- 
 ** 
 ** Abstract: Calculate J1850 CRC	
 ** 
 ** Parameters: Pointer to frame buffer, frame length
 ** 
 ** Returns: CRC of frame
 ** 
 **--------------------------------------------------------------------------- 
 */ 
uint8_t j1850_crc(uint8_t *msg_buf, int8_t nbytes)
{
  uint8_t crc_reg=0xff,poly,byte_count,bit_count;
  uint8_t *byte_point;
  uint8_t bit_point;

  for (byte_count=0, byte_point=msg_buf; byte_count<nbytes; ++byte_count, ++byte_point)
  {
    for (bit_count=0, bit_point=0x80 ; bit_count<8; ++bit_count, bit_point>>=1)
    {
      if (bit_point & *byte_point)	// case for new bit = 1
      {
	if (crc_reg & 0x80)
	  poly=1;	// define the polynomial
	else
	  poly=0x1c;
	crc_reg= ( (crc_reg << 1) | 1) ^ poly;
      }
      else		// case for new bit = 0
      {
	poly=0;
	if (crc_reg & 0x80)
	  poly=0x1d;
	crc_reg= (crc_reg << 1) ^ poly;
      }
    }
  }
  return ~crc_reg;	// Return CRC
}
