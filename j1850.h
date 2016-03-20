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
#ifndef __J1850_H__
#define __J1850_H__

/*** CONFIG START ***/

#define J1850_PORT_OUT	PORTA	// J1850 output port
#define J1850_DIR_OUT 	DDRA	// J1850 direction register
#define J1850_PIN_OUT		2			// J1850 output pin

#define J1850_PORT_IN		PINA	// J1850 input port
#define J1850_PULLUP_IN	PORTA	// J1850 pull-up register
#define J1850_DIR_IN 		DDRA	// J1850 direction register
#define J1850_PIN_IN		0			// J1850 input pin

#define	J1850_PIN_OUT_NEG			// define output level inverted by hardware
#define	J1850_PIN_IN_NEG			// define input level inverted by hardware

/*** CONFIG END ***/

#ifdef J1850_PIN_OUT_NEG
	#define j1850_active() J1850_PORT_OUT &=~ _BV(J1850_PIN_OUT)
	#define j1850_passive() J1850_PORT_OUT |= _BV(J1850_PIN_OUT)
#else
	#define j1850_active() J1850_PORT_OUT |= _BV(J1850_PIN_OUT)
	#define j1850_passive() J1850_PORT_OUT &=~ _BV(J1850_PIN_OUT)
#endif

#ifdef J1850_PIN_IN_NEG
#define is_j1850_active() bit_is_clear(J1850_PORT_IN, J1850_PIN_IN)
#else
#define is_j1850_active() bit_is_set(J1850_PORT_IN, J1850_PIN_IN)
#endif

/* Define Timer0 Prescaler here */
#define c_start_pulse_timer	0x02 // prescaler 8
#define c_stop_pulse_timer	0x00


// define error return codes
#define J1850_RETURN_CODE_UNKNOWN    0
#define J1850_RETURN_CODE_OK         1
#define J1850_RETURN_CODE_BUS_BUSY   2
#define J1850_RETURN_CODE_BUS_ERROR  3
#define J1850_RETURN_CODE_DATA_ERROR 4
#define J1850_RETURN_CODE_NO_DATA    5
#define J1850_RETURN_CODE_DATA       6

// convert microseconds to counter values
#define us2cnt(us) ((unsigned int)((unsigned long)(us) / (1000000L / (float)((unsigned long)MCU_XTAL / 8L))))

#define WAIT_100us	us2cnt(100)		// 100us, used to count 100ms

// define J1850 VPW timing requirements in accordance with SAE J1850 standard
// all pulse width times in us
// transmitting pulse width
#define TX_SHORT	us2cnt(64)		// Short pulse nominal time
#define TX_LONG		us2cnt(128)		// Long pulse nominal time
#define TX_SOF		us2cnt(200)		// Start Of Frame nominal time
#define TX_EOD		us2cnt(200)		// End Of Data nominal time
#define TX_EOF		us2cnt(280)		// End Of Frame nominal time
#define TX_BRK		us2cnt(300)		// Break nominal time
#define TX_IFS		us2cnt(300)		// Inter Frame Separation nominal time

// see SAE J1850 chapter 6.6.2.5 for preferred use of In Frame Respond/Normalization pulse
#define TX_IFR_SHORT_CRC	us2cnt(64)	// short In Frame Respond, IFR contain CRC
#define TX_IFR_LONG_NOCRC us2cnt(128)	// long In Frame Respond, IFR contain no CRC

// receiving pulse width
#define RX_SHORT_MIN	us2cnt(34)	// minimum short pulse time
#define RX_SHORT_MAX	us2cnt(96)	// maximum short pulse time
#define RX_LONG_MIN		us2cnt(96)	// minimum long pulse time
#define RX_LONG_MAX		us2cnt(163)	// maximum long pulse time
#define RX_SOF_MIN		us2cnt(163)	// minimum start of frame time
#define RX_SOF_MAX		us2cnt(239)	// maximum start of frame time
#define RX_EOD_MIN		us2cnt(163)	// minimum end of data time
#define RX_EOD_MAX		us2cnt(239)	// maximum end of data time
#define RX_EOF_MIN		us2cnt(239)	// minimum end of frame time, ends at minimum IFS
#define RX_BRK_MIN		us2cnt(239)	// minimum break time
#define RX_IFS_MIN		us2cnt(280)	// minimum inter frame separation time, ends at next SOF

// see chapter 6.6.2.5 for preferred use of In Frame Respond/Normalization pulse
#define RX_IFR_SHORT_MIN	us2cnt(34)		// minimum short in frame respond pulse time
#define RX_IFR_SHORT_MAX	us2cnt(96)		// maximum short in frame respond pulse time
#define RX_IFR_LONG_MIN		us2cnt(96)		// minimum long in frame respond pulse time
#define RX_IFR_LONG_MAX		us2cnt(163)		// maximum long in frame respond pulse time

uint8_t timeout_multiplier;  // default 4ms timeout multiplier

extern void j1850_init(void);
extern uint8_t j1850_recv_msg(uint8_t *msg_buf );
extern uint8_t j1850_send_msg(uint8_t *msg_buf, int8_t nbytes);
extern uint8_t j1850_crc(uint8_t *msg_buf, int8_t nbytes);

static inline void timer0_start(void)
{
    TCCR0 = c_start_pulse_timer;
    TCNT0 = 0;
}

static inline void timer0_stop(void)
{
    TCCR0 = c_stop_pulse_timer;
}

#endif // __J1850_H__
