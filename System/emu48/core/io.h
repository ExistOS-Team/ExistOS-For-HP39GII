/*
 *   io.h
 *
 *   This file is part of Emu48
 *
 *   Copyright (C) 1999 Christoph Gieﬂelink
 *
 */

// I/O addresses without mapping offset
#define BITOFFSET   0x00					// Display bit offset and DON
#define CRC         0x04					// Crc (16 bit, LSB first)
#define LPD         0x08					// Low Power Detection
#define LPE         0x09					// Low Power detection Enable
#define ANNCTRL		0x0b					// Annunciator Control (2 nibble)
#define BAUD		0x0d					// Baudrate (Bit 2-0)
#define CARDCTL		0x0e					// card control
#define CARDSTAT	0x0f					// card status
#define IOC			0x10					// IO CONTROL
#define RCS			0x11					// RCS
#define TCS			0x12					// TCS
#define CRER		0x13					// CRER
#define RBR_LSB		0x14					// RBR low nibble
#define RBR_MSB		0x15					// RBR high nibble
#define TBR_LSB		0x16					// TBR low nibble
#define TBR_MSB		0x17					// TBR high nibble
#define	SRQ1		0x18					// SRQ1
#define	SRQ2		0x19					// SRQ2
#define IR_CTRL		0x1a					// IR CONTROL
#define LCR			0x1c					// Led Control Register
#define DISP1CTL	0x20					// Display Start Address
#define LINENIBS	0x25					// Display Line Offset
#define LINECOUNT	0x28					// Display Line Counter
#define TIMER1_CTRL	0x2e					// Timer1 Control
#define TIMER2_CTRL	0x2f					// Timer2 Control
#define DISP2CTL	0x30					// Display Secondary Start Address
#define TIMER1		0x37					// Timer1 (4 bit)
#define TIMER2		0x38					// Timer2 (32 bit, LSB first)

// 0x00 Display bit offset and DON [DON OFF2 OFF1 OFF0]
#define DON			0x08					// Display On
#define OFF2		0x04					// Display OFFset Bit2
#define OFF1		0x02					// Display OFFset Bit1
#define OFF0		0x01					// Display OFFset Bit0

// 0x08 Low Power Detection [LB2 LB1 LB0 VLBI]
#define LB2			0x08					// Low Battery indicator memory port 2
#define LB1			0x04					// Low Battery indicator memory port 1
#define LB0			0x02					// Low Battery indicator system battery
#define VLBI		0x01					// Very Low Battery Indicator

// 0x09 Low Power detection Enable [ELBI EVLBI GRAM RST]
#define ELBI		0x08					// Enable Low Battery Indicator
#define EVLBI		0x04					// Enable Very Low Battery Indicator
#define GRAM		0x02					// Glitch sensitive RAM
#define RST			0x01					// ReSeT

// 0x0b Annunciator Control [AON XTRA LA6 LA5] [LA4 LA3 LA2 LA1]
#define AON			0x80					// Annunciators on
#define LXTRA		0x40					// does nothing
#define LA6			0x20					// LA6 - Transmitting
#define LA5			0x10					// LA5 - Busy
#define LA4			0x08					// LA4 - Alert
#define LA3			0x04					// LA3 - Alpha
#define LA2			0x02					// LA2 - ALT Shift
#define LA1			0x01					// LA1 - Shift

// 0x0d SERIAL Baud Rate [UCK BD2 BD1 BD0]
#define UCK			0x08					// Uart ClocK
#define BD2			0x04					// BauDrate Bit2
#define BD1			0x02					// BauDrate Bit1
#define BD0			0x01					// BauDrate Bit0

// 0x0e Card Control [ECDT RCDT SMP SWINT]
#define ECDT		0x08					// Enable Card Detect
#define RCDT		0x04					// Run Card Detect
#define SMP			0x02					// Set module pulled
#define SWINT		0x01					// Software Interrupt

// 0x0f Card Status [P2W P1W P2C P1C]
#define P2W			0x08					// High when Port2 writeable
#define P1W			0x04					// High when Port1 writeable
#define P2C			0x02					// High when Card in Port2 inserted
#define P1C			0x01					// High when Card in Port1 inserted

// 0x10 Serial I/O Control [SON ETBE ERBF ERBZ]
#define SON			0x08					// Serial on
#define ETBE		0x04					// Interrupt on transmit buffer empty
#define ERBF		0x02					// Interrupt on receive buffer full
#define ERBZ		0x01					// Interrupt on receiver busy

// 0x11 Serial Receive Control/Status [RX RER RBZ RBF]
#define RX			0x08					// Rx pin state (read-only)
#define RER			0x04					// Receiver error
#define RBZ			0x02					// Receiver busy
#define RBF			0x01					// Receive buffer full

// 0x12 Serial Transmit Control/Status [BRK LPB TBZ TBF]
#define BRK			0x08					// Break
#define LPB			0x04					// Loopback
#define TBZ			0x02					// Transmitter busy
#define TBF			0x01					// Transmit buffer full

// 0x18 Service Request Register 1 [ISRQ TSRQ USRQ VSRQ]
#define ISRQ		0x08					// IR receiver pulls NINT2
#define TSRQ		0x04					// Timer pulls NINT2
#define USRQ		0x02					// UART pulls NINT2
#define VSRQ		0x01					// VLBI pulls NINT2

// 0x19 Service Request Register 2 [KDN NINT2 NINT LSRQ]
#define KDN			0x08					// Bit set when ON Key or Key Interrupt
#define NINT2		0x04					// State of NINT2
#define NINT		0x02					// State of NINT
#define LSRQ		0x01					// LED driver pulls NINT2

// 0x1a IR Control Register [IRI EIRU EIRI IRE]
#define IRI			0x08					// IR input (read-only)
#define EIRU		0x04					// Enable IR UART mode
#define EIRI		0x02					// Enable IR interrupt
#define IRE			0x01					// IR event

// 0x1c Led Control Register [LED ELBE LBZ LBF]
#define LED			0x08					// Turn on LED
#define ELBE		0x04					// Enable Interrupt on Led Buffer empty
#define LBZ			0x02					// Led Port Busy
#define LBF			0x01					// Led Buffer Full

// 0x28 Display Line Counter LSB [LC3 LC2 LC1 LC0] 
#define LC3			0x08					// LC3 - Line Counter Bit3
#define LC2			0x04					// LC2 - Line Counter Bit2
#define LC1			0x02					// LC1 - Line Counter Bit1
#define LC0			0x01					// LC0 - Line Counter Bit0

// 0x29 Display Line Counter MSB [DA19 M32 LC5 LC4]
#define DA19		0x08					// Drive A[19]
#define M32			0x04					// Multiplex 32 way
#define LC5			0x02					// LC5 - Line Counter Bit5
#define LC4			0x01					// LC4 - Line Counter Bit4

// 0x2e Timer1 Control [SRQ WKE INT XTRA]
#define SRQ			0x08					// Service request
#define WKE			0x04					// Wake up
#define INTR		0x02					// Interrupt
#define XTRA		0x01					// Extra function

// 0x2f Timer2 Control [SRQ WKE INT RUN]
#define SRQ			0x08					// Service request
#define WKE			0x04					// Wake up
#define INTR		0x02					// Interrupt
#define RUN			0x01					// Timer run
