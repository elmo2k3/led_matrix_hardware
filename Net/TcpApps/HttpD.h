/*
 * Important defines and function prototypes for the HTTP Daemon Implementation
 * 
 * Author: Simon Kueppers
 * Email: simon.kueppers@web.de
 * Homepage: http://klinkerstein.m-faq.de
 * 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 Copyright 2008 Simon Kueppers
 * */

#ifndef HTTP_H_
#define HTTP_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#define PORT_HTTPD	8080

struct tHttpD
{
	uint8_t nState;
	const prog_void* pData;
	uint32_t nDataLeft;
	uint8_t nNewlines;
	uint8_t nParseLeft;
	uint8_t ParseCmd;
	uint8_t ParseNum;
	uint8_t ParseState;
	uint16_t nPrevBytes;
};

void HttpDInit();
void HttpDCall(	uint8_t* pBuffer,
				uint16_t nBytes,
				struct tHttpD* pSocket);

#endif /*HTTP_H_*/
