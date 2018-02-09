/*
 * Copyright (c) 2017, arneri, arneri@ukr.net All rights reserved
 *
 * A 'client' part of HTTP-JSON task
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *          notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *          notice, this list of conditions and the following disclaimer in the
 *          documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *          the names of its contributors may be used to endorse or promote
 *          products derived from this software without specific prior written
 *          permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Include runtime substitutes: forged std. functions, defines, et al */
#include "client.h"

#include "json/json.h"



/* Error message output and abnormal termination of 'client' */
static void error(const char *msg)
{
#ifndef NO_POSIX
	/* Verbose a problem occured */
	perror(msg);
#else
	; /* TODO: self explanatory */
#endif /* NO_POSIX */

	/* Leave the program */
	exit(0);

} /* void error(. . .) */


/* A test JSON budnle, will be altered while the task procesisng */
static char JSON_STRING[] =
" {\"port\": { \" portID \" : 1, \" time \" : 0x00000000,  \" current_value \" : 0, \" recent_values \" : [ 0, 0, 0 ]}, \" port \" : { \" portID \" : 2, \" time \" : 0x00000000,  \" current_value \" : 0, \" recent_values \" : [ 0, 0, 0 ]} } ";

static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0)

		return 0;

	return -1;
}

#ifdef NO_POSIX
/* TODO: remove once a function pointer will be assigned to other function */
void vIPv4Handler(tcp_Socket * s, unsigned char * dp, int len)
{	
	if (len)
	{
		printf ("Comp. A has received the following stuff: [");
		while (len--)
			printf ("%c", *dp++);
		printf ("]\n");
	}

} /* void vIPv4Handler(. . .) */
#endif /* NO_POSIX */

/* Handler entry point  */
int iXmit(char * arg_three, char * arg_four, char * arg_five)
{
/* Cycle counter */
int iIdx;

/* Server port ID */
int iPortNo; 

/* Hostname (almost useless) */
char * pcHost; 

#ifndef NO_POSIX
/* Server info structure */
struct hostent *heServer;

/* IPV4 socket structure */
struct sockaddr_in saServerAddr;
#else

#endif /* NO_POSIX */


/* Socket ID */ 
int iSockFd;

/* Sent and received counters */
int iBytes, iSent, iReceived, iTotal, iMessageSize;

/* An allocatble buffer pointer to compose a responce in */
char *pcMessage;

/* Arrays to allocate a responce */
char mResponse[RCV_BUFSIZE];

	/* Assign default one */
	iPortNo = DEFAULT_PORT;

	/* Assign default one */
	pcHost = DEFAULT_HOST;

	/* Assign default value to an intermediate 'iMessageSize' prior to actual processing */
	iMessageSize = 0;

	/* GET request. Compliant to <RFC 2616> */
	if ( !strcmp(arg_three, "GET") || !strcmp(arg_three, "HEAD") )
	{
		/* Compose method port */
		iMessageSize += strlen("%s %s%s%s HTTP/1.0\r\n");

		/* Compose path part */
		iMessageSize += strlen(arg_three);

		/* Compose headers part */
		iMessageSize += strlen(arg_four);

		/* blank line */
		iMessageSize += strlen("\r\n");                          
	}
	else
	{
		/* Compose HDR part */
		iMessageSize += strlen("%s %s HTTP/1.0\r\n");

		/* Compose method part */
		iMessageSize += strlen(arg_three);

		/* Compose path part */
		iMessageSize += strlen(arg_four);

		/* Compose 'C-L' part */
		iMessageSize += strlen("Content-Length: %d\r\n") + 10 /* TODO: verbose this '10' or remove at all. */;

		/* blank line */
		iMessageSize += strlen("\r\n");                          

		/* Append a body */
		iMessageSize += strlen(arg_five);
	}

#ifndef NO_POSIX
	/* Allocate space for the message */
	pcMessage=malloc(iMessageSize);
#else
	/* Take an empty area big enough to do the deeds */
	pcMessage = (char *)PLATFORMS_FREE_SPACE; 
#endif /* NO_POSIX */

	/* GET request. Compliant to <RFC 2616> */
	if ( !strcmp(arg_three, "GET") || !strcmp(arg_three, "HEAD") )
	{
		/* Fill in parameters: proto ID, method and path delimiter '/' */
		sprintf(pcMessage,"%s %s HTTP/1.0\r\n",	strlen(arg_three) > 0 ? arg_three :"GET" , strlen(arg_four) > 0 ? arg_four:"/");

		/* Finally add an empty line */
		strcat(pcMessage,"\r\n");
	}
	else
	{
		/* Compose PFX part, method and path */
		sprintf(pcMessage,"%s %s HTTP/1.0\r\n", strlen(arg_three) > 0 ? arg_three : "POST", strlen(arg_four) > 0 ? arg_four : "/");

		/* Compose a 'C-L' part */
		sprintf(pcMessage + strlen(pcMessage), "Content-Length: %d\r\n",strlen( arg_five ) );

		/* Add blank line */
		strcat(pcMessage,"\r\n");

		/* Form body */
		strcat(pcMessage, arg_five );
	}

#ifndef NO_POSIX
	/* Add what is requested */
	printf("Request:\n%s\n",pcMessage);
#else
	;
#endif /* NO_POSIX */


#ifndef NO_POSIX

	/* Create the socket */
	iSockFd = socket(AF_INET, SOCK_STREAM, 0);

	/* Socket not created? */
	if (0 > iSockFd)

		/* Exit with error */
		error("ERROR opening socket");

	/* Assign server an IPv4 address  */
	heServer = gethostbyname(pcHost);

	/* Address is not correct */
	if (NULL == heServer)

		/* Exit with error */
		error("ERROR, no such host");

	/* Fill in the server address structure */
	memset(&saServerAddr, 0, sizeof(saServerAddr));

	/* Assign an address type */
	saServerAddr.sin_family = AF_INET;

	/* Assign a port */
	saServerAddr.sin_port = htons(iPortNo);

	/* Assign an address */
	memcpy(&saServerAddr.sin_addr.s_addr,heServer->h_addr,heServer->h_length);

	/* Try to connect to a socket */
	if (0 > connect(iSockFd,(struct sockaddr *)&saServerAddr,sizeof(saServerAddr))  )

		/* Exit with error once not connected */
		error("ERROR connecting");

	/* Send a request */
	iTotal = strlen(pcMessage);

	/* Default the counter of bytes sent */
	iSent = 0;

	do
	{
		/* Try to send some of the data */
		iBytes = write(iSockFd, pcMessage + iSent, iTotal - iSent);

		/* Can't send? */
		if (0 > iBytes)

			/* Exit with error */
			error("ERROR writing message to socket");

		/* Nothing to send? */
		if (0 == iBytes)

			/* Leave the loop */
			break;

		/* Increase the counter of bytes sent on amount of actually sent bytes */
		iSent += iBytes;

	/* Keep on doing the above as long as we've got somehting to sent in the buffer */
	} while (iSent < iTotal);

	/* Clear the receive buffer  */
	memset(mResponse, 0, sizeof(mResponse));

	/* Set intermediate value to a max. length of the rcv. buffer */
	iTotal = sizeof(mResponse)-1;

	/* Counter of bytes received to be assigend to zero */
	iReceived = 0;

	do
	{
		/* Try to read some bytes from a socket */
		iBytes = read(iSockFd, mResponse + iReceived, iTotal - iReceived);

		/* Can't receive? */
		if (0 > iBytes)

			/* Exit with error */
			error("ERROR reading response from socket");

		/* Nothing to receive? */
		if (0 == iBytes)

			/* Leave a loop */
			break;

		/* Increase the counter of bytes received on amount of actually received bytes */
		iReceived += iBytes;

	/* Keep on doing the above as long as we've got somehting to sent in the buffer */
	} while (iReceived < iTotal);

	/* Amount of bytes received exceeds buffer limit? */
	if (iReceived == iTotal)

		/* Exit with error */
		error("ERROR storing complete response from socket");

	/* Close a socket */
	close(iSockFd);

	/* Process a response */
	printf("Response:\n%s\n", mResponse);

	/* Dispose a buffer allocated */
	free(pcMessage);

#else
	/* 0x800 == 1500 bytes - for TCP payload, rest - for Eth, IP, and TCP headers; ensure it does not affect the vital mem. areas on embedded platf. */
	floating_socket_buffer = (unsigned char *)(PLATFORMS_FREE_SPACE - 0x800);

	/* Initialize the socket's API: APR-broadcasts and logging status */
	tcp_Module_Init();

	/* Skip Ethernet header - point onto IP directly */
	floating_ip_buffer = (in_Header * )sed_IsPacket();

	/* Initialize socket data on local endpoint */
	tcp_Init(AlfaEthAddress, AlfaIpAddress);

#if 0
 	Move((unsigned char*)(&(AlfaEthAddress[0])), (unsigned char*)(&(sed_lclEthAddr[0])), sizeof(eth_HwAddress));
#endif /* 0 */

	/* Assign local IP address structure */
    	sin_lclINAddr = AlfaIpAddress;

	/* Put local socket into default state */
	tcp_allsocs = onAlfa;

	/* Define that first (and only) 'IPv4Handler()' handler will be deployed */
	tcp_id = idOnAlfa;

#if 0
	__tcp_DumpHeader();
#endif /* 0 */

	/* Opening this socket for active writing */
	tcp_Open(&Socket1, ushAlfaPort, BetaIpAddress, ushBetaPort, vIPv4Handler);


	/* TODO: do the receives and transfers as long as external handler 'IPv4Handler()' does none */

	tcp_Close(&Socket1);

#endif /* NO_POSIX */

	/* Transfer is done, socket is closed */
	return 0;

} /* int iXmit(. . .) */

/* Receive a character from UART. Return this character 'as is' */
static unsigned char SerialRecvChar(int iFlag)
{
	/* Forge char of UART */
	return 'A';

} /* static unsigned char SerialRecvChar(. . .) */


/* Send a character to UART. Variable 'iFlag' - sending flags - not used by now  */
static void SerialSendChar(int iFlag, unsigned char uchChar)
{
#ifndef NO_POSIX
	/* Send this back to an UART */
	printf("\nSending this 'char' to UART ++++++>%c<+++++++++\n", uchChar);
#else
	;
#endif /* NO_POSIX */
	return;

} /* static void SerialSendChar(. . .) */


/* local function to provide a simple interpreter across a serial line */
static void KernelBasicLoop( )
{
/* Temporary variable to store a character received from UART */
unsigned char uchChar;

char cBuffer[MAIN_BUF_LEN];

int i, r;

jsmn_parser p;

jsmntok_t t[JSON_MAX_TKNS];

	/* Do the loop endlessly */
	for ( ; ; )
	{
		/* Get a character from UART */
		uchChar = SerialRecvChar(0);

		/* Form a responce string. TODO: 1) replace fake data '8' with real one; 2) don't use absolute array index '72', use field name instead */
		( (char*) JSON_STRING)[72]	= '8';

		/* Post this character to HTTP-server within JSON-envelope */
		iXmit("POST", "local", (char*)JSON_STRING);

#ifndef NO_POSIX
		/* Sleep for while: time is needed for UART to stabilze, ethernet PHYs to stabilize, etc */
		usleep(UTMO_UART);
#else
		; /* TODO: self explanatory */
#endif /* NO_POSIX */

		/* TODO: can we drag it outside the cycle? */
		jsmn_init(&p);

		r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), t, sizeof(t)/sizeof(t[0]));

		if (r < 0)
		{
#ifndef NO_POSIX
			printf("Failed to parse JSON: %d\n", r);
#else
			;
#endif /* NO_POSIX */

			/* Hopefully the next received HTTP-data will be in JSON format and parsable */
			continue;
		}

		/* Assume the top-level element is an object */
		if (r < 1 || t[0].type != JSMN_OBJECT)
		{
#ifndef NO_POSIX
			printf("Object expected\n");
#else
			;
#endif /* NO_POSIX */

			/* Hopefully the next received HTTP-data will contain no illegal characters, strings, integers */
			continue;
		}

		/* Loop over all keys of the root object */
		for (i = 1; i < r; i++)
		{
			if (jsoneq(JSON_STRING, &t[i], "port") == 0)
			{
			int j, i2do = 0;

				for (j = 0; j < 2*t[i+1].size; j++)
				{
				char pcTmp[MAIN_BUF_LEN];

					jsmntok_t *g = &t[i+j+2];

#ifndef NO_POSIX
					printf("  * %.*s\n", g->end - g->start, JSON_STRING + g->start);
#else
					;
#endif /* NO_POSIX */


#ifndef NO_POSIX
					sprintf(pcTmp, "%.*s", g->end - g->start, JSON_STRING + g->start);
#else
					; /* TODO: self explanatory */
#endif /* NO_POSIX */

					

					/* Token has been already found - it's time to do a HTTP->UART transfer */
					if (i2do)
					{
						/* Avoid processing same tocket more and more */
						i2do = 0;

						/* Post it back */
						SerialSendChar(0, pcTmp[0] );
					}

					/* Blank space character on begin of a string is a product of earlier 'sprintf' transform. */
					if ( NULL != strstr((char*) pcTmp, " current_value") )

						/* Define that a desired tocken has been found within current JSON envelope */
						i2do = 1;
				}

				i += t[i+1].size + 1;
			}
		}

	}

	/* Normally unreachable but who knows how the loops are terminated on embedded platform 'f4' */
	return;

} /* static void KernelBasicLoop() */



int main(int argc,char *argv[])
{
	/* Endlessly do the main loop of routine */
	KernelBasicLoop();

	/* Unreachanble */
	return EXIT_SUCCESS;

} /* main(. . .) */
