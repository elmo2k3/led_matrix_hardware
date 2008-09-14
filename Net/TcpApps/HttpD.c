/*
 * Code file containing the HTTP Daemon module.
 * It supports GET (not completely implemented) and POST Request methods
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

#include <avr/pgmspace.h>
#include "HttpD.h"
#include "../uip/uip.h"
#include "../../Hardware/Gpio.h"

#define STATE_CONNECTED		0	//Client has just connected
#define STATE_GET_G			1	//G
#define STATE_GET_GE		2	//GE
#define STATE_GET_GET		3	//GET
#define STATE_POST_P		4	//P
#define STATE_POST_PO		5	//PO
#define STATE_POST_POS		6	//POS
#define STATE_POST_POST		7	//POST
#define STATE_GOTGET		8	//Client just sent a GET request
#define STATE_GOTPOST		9	//Client just sent a POST request
#define STATE_PARSEPOST		10	//we are currently parsing the client's POST-data
#define STATE_SENDHEADER	11	//next we send him the HTTP header
#define STATE_SENDDATA		12	//followed by data

#define PARSEBYTES			167	//stop parsing POST data after 167 bytes
#define PARSE_CMD			0
#define PARSE_NUM10			1
#define PARSE_NUM1			2
#define PARSE_EQUAL			3
#define PARSE_VAL			4
#define PARSE_DELIM			5

static const prog_char
		g_HtmlPageDefault[] =
				"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">"
					"<html><head>"
					"<title>uWebSrv</title>"
					"<style type='text/css'>"
					".s0 { background-color: red; }"
					".s1 { background-color: green; }"
					"td { text-align: center; }"
					"</style>"
					"</head><body>"
					"<h1>uWebSrv</h1>"
					"<form method='POST' action='/'>"
					"<table border='1px'><colgroup><col width='100px'><col width='40px'><col width='40px'><col width='40px'></colgroup>"
					"<tr><td></td><td>PIN</td><td>PORT</td><td>DDR</td></tr>"
					"<tr><td>GPIO 00</td><td class='s%i00'>%i00</td><td class='s%o00'><input type='text' name='o00' value='%o00' maxlength='1' size='1'></td><td class='s%d00'><input type='text' name='d00' value='%d00' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 01</td><td class='s%i01'>%i01</td><td class='s%o01'><input type='text' name='o01' value='%o01' maxlength='1' size='1'></td><td class='s%d01'><input type='text' name='d01' value='%d01' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 02</td><td class='s%i02'>%i02</td><td class='s%o02'><input type='text' name='o02' value='%o02' maxlength='1' size='1'></td><td class='s%d02'><input type='text' name='d02' value='%d02' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 03</td><td class='s%i03'>%i03</td><td class='s%o03'><input type='text' name='o03' value='%o03' maxlength='1' size='1'></td><td class='s%d03'><input type='text' name='d03' value='%d03' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 04</td><td class='s%i04'>%i04</td><td class='s%o04'><input type='text' name='o04' value='%o04' maxlength='1' size='1'></td><td class='s%d04'><input type='text' name='d04' value='%d04' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 05</td><td class='s%i05'>%i05</td><td class='s%o05'><input type='text' name='o05' value='%o05' maxlength='1' size='1'></td><td class='s%d05'><input type='text' name='d05' value='%d05' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 06</td><td class='s%i06'>%i06</td><td class='s%o06'><input type='text' name='o06' value='%o06' maxlength='1' size='1'></td><td class='s%d06'><input type='text' name='d06' value='%d06' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 07</td><td class='s%i07'>%i07</td><td class='s%o07'><input type='text' name='o07' value='%o07' maxlength='1' size='1'></td><td class='s%d07'><input type='text' name='d07' value='%d07' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 08</td><td class='s%i08'>%i08</td><td class='s%o08'><input type='text' name='o08' value='%o08' maxlength='1' size='1'></td><td class='s%d08'><input type='text' name='d08' value='%d08' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 09</td><td class='s%i09'>%i09</td><td class='s%o09'><input type='text' name='o09' value='%o09' maxlength='1' size='1'></td><td class='s%d09'><input type='text' name='d09' value='%d09' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 10</td><td class='s%i10'>%i10</td><td class='s%o10'><input type='text' name='o10' value='%o10' maxlength='1' size='1'></td><td class='s%d10'><input type='text' name='d10' value='%d10' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 11</td><td class='s%i11'>%i11</td><td class='s%o11'><input type='text' name='o11' value='%o11' maxlength='1' size='1'></td><td class='s%d11'><input type='text' name='d11' value='%d11' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 12</td><td class='s%i12'>%i12</td><td class='s%o12'><input type='text' name='o12' value='%o12' maxlength='1' size='1'></td><td class='s%d12'><input type='text' name='d12' value='%d12' maxlength='1' size='1'></td></tr>"
					"<tr><td>GPIO 13</td><td class='s%i13'>%i13</td><td class='s%o13'><input type='text' name='o13' value='%o13' maxlength='1' size='1'></td><td class='s%d13'><input type='text' name='d13' value='%d13' maxlength='1' size='1'></td></tr>"
					"</table>"
					"<input type='submit' value='Modify'>"
					"</form>"
					"</body></html>";

static uint16_t CopyStringP(uint8_t** ppBuffer,
							const prog_char* pString)
{
	uint16_t nBytes = 0;
	char Character;

	while ((Character = pgm_read_byte(pString)) != '\0')
	{
		**ppBuffer = Character;
		*ppBuffer = *ppBuffer + 1;
		pString = pString + 1;
		nBytes++;
	}

	return nBytes;
}

static uint16_t CopyValue(	uint8_t** ppBuffer,
							uint32_t nValue)
{
	static PROGMEM int32_t nDecTab[] =
	{
		1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10,
				1, 0 };

	int32_t nDec;
	uint8_t nCurTabIndex = 0;
	char cNumber;
	uint8_t nBytes = 0;

	while ((nDec = pgm_read_dword(&nDecTab[nCurTabIndex])) != 0)
	{
		for (cNumber='0'; cNumber<'9'; cNumber++)
		{
			int32_t nTmp = nValue - nDec;

			if (nTmp < 0)
				break;

			nValue = nTmp;
		}

		**ppBuffer = cNumber;
		*ppBuffer = *ppBuffer + 1;
		nBytes++;
		nCurTabIndex++;
	}

	return nBytes;
}

static uint16_t CopyHttpHeader(	uint8_t* pBuffer,
								uint32_t nDataLen)
{
	uint16_t nBytes = 0;

	nBytes += CopyStringP(&pBuffer, PSTR("HTTP/1.1 200 OK"));
	nBytes += CopyStringP(&pBuffer, PSTR("\r\n"));

	nBytes += CopyStringP(&pBuffer, PSTR("Content-Length:"));
	nBytes += CopyValue(&pBuffer, nDataLen);
	nBytes += CopyStringP(&pBuffer, PSTR("\r\n"));

	nBytes += CopyStringP(&pBuffer, PSTR("Content-Type:text/html\r\n"));
	nBytes += CopyStringP(&pBuffer, PSTR("Connection:close\r\n"));
	nBytes += CopyStringP(&pBuffer, PSTR("\r\n"));

	return nBytes;
}

static uint16_t CopyHttpData(	uint8_t* pBuffer,
								const prog_void** ppData,
								uint32_t* pDataLeft,
								uint16_t nMaxBytes)
{
	uint16_t nBytes = 0;
	uint8_t nByte;
	uint8_t nParsedNum;
	uint8_t nParsedMode;

	while (nMaxBytes--)
	{
		if (*pDataLeft > 0)
		{
			nByte = pgm_read_byte(*ppData);

			if (nByte == '%')
			{
				*ppData = *ppData + 1;
				*pDataLeft = *pDataLeft - 1;
				nParsedMode = pgm_read_byte(*ppData);
				*ppData = *ppData + 1;
				*pDataLeft = *pDataLeft - 1;
				nParsedNum = (pgm_read_byte(*ppData) - '0') * 10;
				*ppData = *ppData + 1;
				*pDataLeft = *pDataLeft - 1;
				nParsedNum += (pgm_read_byte(*ppData) - '0') * 1;

				if (nParsedMode == 'i')
					*pBuffer = GpioGetPin(nParsedNum) + '0';
				else if (nParsedMode == 'o')
					*pBuffer = GpioGetPort(nParsedNum) + '0';
				else if (nParsedMode == 'd')
					*pBuffer = GpioGetDdr(nParsedNum) + '0';
			}
			else
				*pBuffer = nByte;

			*ppData = *ppData + 1;
			*pDataLeft = *pDataLeft - 1;
			pBuffer++;
			nBytes++;

		}
		else
			break;
	}

	return nBytes;
}

void HttpDInit()
{
	//Start listening on our port
	uip_listen(HTONS(PORT_HTTPD));
}

void HttpDCall(	uint8_t* pBuffer,
				uint16_t nBytes,
				struct tHttpD* pSocket)
{
	uint16_t nBufSize;

	if (uip_connected())
	{
		//Initialize this connection
		pSocket->pData = g_HtmlPageDefault;
		pSocket->nDataLeft = sizeof(g_HtmlPageDefault)-1;
		pSocket->nNewlines = 0;
		pSocket->nState = STATE_CONNECTED;
		pSocket->nPrevBytes = 0xFFFF;
	}
	else if (uip_newdata() || uip_acked())
	{
		if (pSocket->nState == STATE_CONNECTED)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == 'G')
				pSocket->nState = STATE_GET_G;
			else if (*pBuffer == 'P')
				pSocket->nState = STATE_POST_P;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_GET_G)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == 'E')
				pSocket->nState = STATE_GET_GE;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_GET_GE)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == 'T')
				pSocket->nState = STATE_GET_GET;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_GET_GET)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == ' ')
				pSocket->nState = STATE_GOTGET;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_POST_P)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == 'O')
				pSocket->nState = STATE_POST_PO;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_POST_PO)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == 'S')
				pSocket->nState = STATE_POST_POS;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_POST_POS)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == 'T')
				pSocket->nState = STATE_POST_POST;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_POST_POST)
		{
			if (nBytes == 0)
				return;
			if (*pBuffer == ' ')
				pSocket->nState = STATE_GOTPOST;
			nBytes--;
			pBuffer++;
		}

		if (pSocket->nState == STATE_GOTPOST || pSocket->nState == STATE_GOTGET)
		{
			//Search for \r\n\r\n
			while (nBytes != 0)
			{
				if (*pBuffer == '\n')
				{
					pSocket->nNewlines++;
				}
				else if (*pBuffer == '\r')
				{
				}
				else
				{
					pSocket->nNewlines = 0;
				}

				pBuffer++;
				nBytes--;

				if (pSocket->nNewlines == 2)
				{
					//beginning found.
					if (pSocket->nState == STATE_GOTPOST)
					{
						//Initialize Parsing variables
						pSocket->nParseLeft = PARSEBYTES;
						pSocket->ParseState = PARSE_CMD;
						//start parsing
						pSocket->nState = STATE_PARSEPOST;
					}
					else if (pSocket->nState == STATE_GOTGET)
					{
						pSocket->nState = STATE_SENDHEADER;
					}
					break;
				}
			}
		}

		if (pSocket->nState == STATE_PARSEPOST)
		{
			while (nBytes--)
			{
				if (pSocket->ParseState == PARSE_CMD)
				{
					pSocket->ParseCmd = *pBuffer;
					pSocket->ParseState = PARSE_NUM10;
				}
				else if (pSocket->ParseState == PARSE_NUM10)
				{
					pSocket->ParseNum = (*pBuffer - '0') * 10;
					pSocket->ParseState = PARSE_NUM1;
				}
				else if (pSocket->ParseState == PARSE_NUM1)
				{
					pSocket->ParseNum += (*pBuffer - '0');
					pSocket->ParseState = PARSE_EQUAL;
				}
				else if (pSocket->ParseState == PARSE_EQUAL)
				{
					pSocket->ParseState = PARSE_VAL;
				}
				else if (pSocket->ParseState == PARSE_VAL)
				{
					if (pSocket->ParseCmd == 'd')
						GpioSetDdr(pSocket->ParseNum, *pBuffer - '0');
					else
						GpioSetPort(pSocket->ParseNum, *pBuffer - '0');

					pSocket->ParseState = PARSE_DELIM;
				}
				else if (pSocket->ParseState == PARSE_DELIM)
				{
					pSocket->ParseState = PARSE_CMD;
				}

				pSocket->nParseLeft--;
				pBuffer++;

				if (pSocket->nParseLeft == 0)
				{
					//finished parsing
					pSocket->nState = STATE_SENDHEADER;
					break;
				}
			}
		}

		if (pSocket->nState == STATE_SENDHEADER)
		{
			uip_send(uip_appdata, CopyHttpHeader(uip_appdata, pSocket->nDataLeft));
			pSocket->nState = STATE_SENDDATA;
			return;
		}

		if (pSocket->nState == STATE_SENDDATA)
		{
			//We have sent the HTML Header or HTML Data previously.
			//Now we send (further) Data depending on the Socket's pData pointer
			//If all data has been sent, we close the connection
			pSocket->nPrevBytes = pSocket->nDataLeft;
			nBufSize
					= CopyHttpData(uip_appdata, &pSocket->pData, &pSocket->nDataLeft, uip_mss());
			pSocket->nPrevBytes -= pSocket->nDataLeft;
			
			
			if (nBufSize == 0)
			{
				//No Data has been copied. Close connection
				uip_close();
			}
			else
			{
				//Else send copied data
				uip_send(uip_appdata, nBufSize);
			}
			return;
		}
	}
	else if (uip_rexmit())
	{
		if (pSocket->nPrevBytes == 0xFFFF)
		{
			/* Send header again */
			uip_send(uip_appdata, CopyHttpHeader(uip_appdata, pSocket->nDataLeft));
		}else
		{
			pSocket->pData -= pSocket->nPrevBytes;
			pSocket->nDataLeft += pSocket->nPrevBytes;
			pSocket->nPrevBytes = pSocket->nDataLeft;
			nBufSize
					= CopyHttpData(uip_appdata, &pSocket->pData, &pSocket->nDataLeft, uip_mss());
			pSocket->nPrevBytes -= pSocket->nDataLeft;
			
			if (nBufSize == 0)
			{
				//No Data has been copied. Close connection
				uip_close();
			}
			else
			{
				//Else send copied data
				uip_send(uip_appdata, nBufSize);
			}
		}
		return;
	}

}
