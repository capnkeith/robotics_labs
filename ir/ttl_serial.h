//******************************************************************************
// Copyright 2022 Seth Keith, all rights reserved.
//******************************************************************************

#ifndef __TTL_SERIAL_H__
#define __TTL_SERIAL_H__

uint8_t uart_receive( void );
void uart_send_string(char *string);
void uart_init( uint16_t ubrr);     
void uart_send( char data );

#endif // __TTL_SERIAL_H__
