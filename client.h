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

#ifndef _CLIENT_H_
#define _CLIENT_H_

/* Size of buffer to contain a respoince. */
#define RCV_BUFSIZE	4096

/* Port on which HTTP server will be started.  */
#define DEFAULT_PORT	88

/* Target host IP */
#define DEFAULT_HOST	"127.0.0.1"

/* Microseconds to wait between RCEV and SEND on universal asynchronous adapter */
#define UTMO_UART	2000000

/* Upper limit of JSON tockes. Actually could be a 4 or eben 2, but... */
#define JSON_MAX_TKNS	64

/* Microbuffer length. Shoudl fit on the stack of ANY campiler's executable */
#define MAIN_BUF_LEN	0x100


#ifndef NO_POSIX

/* printf(), sprintf(), etc */
#include <stdio.h> 

/* exit(), atoi(), malloc(), free(), etc */
#include <stdlib.h> 

/* read(), write(), close() */
#include <unistd.h> 

/* memcpy(), memset() */
#include <string.h> 

/* socket(), connect() */
#include <sys/socket.h> 

/* Types 'struct sockaddr_in', 'struct sockaddr' */
#include <netinet/in.h> 

/* Type 'struct hostent' and gethostbyname() */
#include <netdb.h> 

#else

#include "tcp/tcp.h"

/* Redefine for we don't have it outside CLIBC runtime */
#define		EXIT_SUCCESS			0

/* Platform's free space. Should come via LD-scepit, as an option */
#define		PLATFORMS_FREE_SPACE		0xFFFFFFFF

/* Forge. TODO: replace with target's implementation */
int sprintf(char *str, const char *format, ...)
{
	return -1;

} /* int sprintf(. . .)*/

/* Forge. TODO: replace with target's implementation */
char *strncat(char *dest, const char *src, int n)
{
	return (char *)0;

} /* char *strncat(. . .)*/

/* Forge. TODO: replace with target's implementation */
char *strcat(char *dest, const char *src)
{
	return (char *)0;

} /* char *strcat(. . .)*/

/* Forge. TODO: replace with target's implementation */
int strlen(const char *s)
{
	return 0;

} /* int strlen(. . ) */

/* Forge. TODO: replace with target's implementation */
void memset(void *s, int c, int n)
{
char * pcS;

	pcS = (char *)s;

	while(n--)

	    *pcS++ = c;

} /* void memset(. . .)*/

/* Forge. TODO: replace with target's implementation */
int strncmp(char * cs, char * ct, unsigned int count0)
{
char *pcSu1, *pcSu2;

signed char iRes = 0;	

	for(pcSu1 = cs, pcSu2 = ct; 0 < count0; ++pcSu1, ++pcSu2, count0--)

		if ((iRes = *pcSu1 - *pcSu2) != 0)

			break;

	/* Non-zero means that starting from certain position strings differ, zero - equal  */
	return iRes;

} /* int strncmp(. . .) */

/* Forge. TODO: replace with target's implementation */
char * strstr(char *s1, char *s2)
{
	return (char *)0;

} /* char * strstr(. . .) */

/* Forge. TODO: replace with target's implementation */
void exit(int c)
{
	/* TODO: set err code 'c' */;

	for (;;);

	/* Unreachable */
	return;

} /* void exit(. . .) */

#endif /* (#ifndef NO_POSIX) */

#endif /* _CLIENT_H_ */
