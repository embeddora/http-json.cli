/*
 * Copyright (c) 2017, arneri, arneri@ukr.net All rights reserved
 *
 * A 'client' part of task
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
 */

/* Own interface */
#include "tcp.h"


static long clock_MS;

unsigned char * floating_socket_buffer;

/* A second endpoint of the socket - on the server */
tcp_Socket Socket1 /*, Socket2*/;

/* ushAlfaPort - client's port, basically any;  'ushBetaPort' - server port, same as 'DEFAULT_PORT' */
unsigned short ushAlfaPort = 5089, ushBetaPort = 89;

/* AlfaIpAddress - client address, whatever is appropriate; BetaIpAddress - server address, same as 'DEFAULT_HOST' */
in_HwAddress   AlfaIpAddress = 0x10000001,  BetaIpAddress = 0x1000001; 

static eth_HwAddress  AlfaEthAddress = {0xEA21, 0xBA31, 0xFA41}, BetaEthAddress = {0x1133, 0x5544, 0x9977};

static in_Header * floating_ip_buffer;

/* IP identification number(s) */
int tcp_id, idOnAlfa/*, idOnBeta*/;

/* Socket chains */
tcp_Socket *tcp_allsocs, *onAlfa/*, *onBeta*/;

static eth_HwAddress sed_lclEthAddr, sed_ethBcastAddr;

static in_HwAddress  sin_lclINAddr;

#ifdef DEBUG_INFO

static unsigned short tcp_logState;

#endif /* DEBUG_INFO */


#define MoveW(a, b, c) memcpy((void*)b, (void*)a, (int)c)





void tcp_Module_Init(void)
{   
int i;    

	for (i=0; i<3; i++)

		sed_ethBcastAddr[i] = 0xFFFF;

#ifdef DEBUG_INFO

	tcp_logState = 0;

#endif /* DEBUG_INFO */

} /* void tcp_Module_Init() */

/* Initialize the tcp implementation on given machine */
void tcp_Init(eth_HwAddress _MyEthHwAddress, unsigned long uloMyIpAddress)
{
    /* on this machine we need it (again) */
    tcp_allsocs = NULL;

    /* initialize ethernet interface */
    sed_lclEthAddr[0] = _MyEthHwAddress[0];

    sed_lclEthAddr[1] = _MyEthHwAddress[1];

    sed_lclEthAddr[2] = _MyEthHwAddress[2];

    tcp_id = 0;

    /* Hack: assume the network number */
    sin_lclINAddr = uloMyIpAddress;

} /* void tcp_Init(. . .)*/


/* Actively open a TCP connection to a particular destination. */
unsigned long tcp_Open(tcp_Socket *s, unsigned short lport, in_HwAddress ina, unsigned short port, procref datahandler)
{
	s->state = tcp_StateSYNSENT;

	s->timeout = tcp_LONGTIMEOUT;

	if ( lport == 0 )
	{
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf ("ATTENTION: targ. port = 0. Can't proceed.\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */

		return 0;
	}
 
	s->myport = lport;

	/* ARP stuff is going to be reconciliated as soon as i get the 803.2 hardware */
	if ( ! sar_MapIn2Eth(ina, &s->hisethaddr[0]) )
	{
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Open of 0x%x: defaulting ethernet address to broadcast\n", ina);
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		Move(&sed_ethBcastAddr[0], &s->hisethaddr[0], sizeof(eth_HwAddress));
	}

	s->hisaddr = ina;

	s->hisport = port;

	s->seqnum = 0;

	s->dataSize = 0;

	s->flags = tcp_FlagSYN;

	s->unhappy = true;

	s->dataHandler = datahandler;

	s->next = tcp_allsocs;

	tcp_allsocs = s;

	tcp_Send(s);

} /* unsigned long tcp_Open() */

/* Passive open: listen for a connection on a particular port */
static void tcp_Listen(tcp_Socket *s, unsigned short port, procref datahandler, int timeout )
{
	s->state = tcp_StateLISTEN;

	if ( timeout == 0 )

	 	/* Do the job forever */
		s->timeout = 0x7ffffff;

	else s->timeout = timeout;

	s->myport = port;

	s->hisport = 0;

	s->seqnum = 0;

	s->dataSize = 0;

	s->flags = 0;

	s->unhappy = 0;

	s->dataHandler = datahandler;

	s->next = tcp_allsocs;

	tcp_allsocs = s;

} /* void tcp_Listen(. . .) */

/* Send a FIN on a particular port -- only works if it is open */
void tcp_Close(tcp_Socket * s)
{
	if ( s->state == tcp_StateESTAB || s->state == tcp_StateSYNREC )
	{
		s->flags = tcp_FlagACK | tcp_FlagFIN;

		s->state = tcp_StateFINWT1;

		s->unhappy = true;
	}

} /* void tcp_Close(... )*/

/* Abort a tcp connection */
static void tcp_Abort(tcp_Socket *s)
{
	if ( s->state != tcp_StateLISTEN && s->state != tcp_StateCLOSED )
	{
		s->flags = tcp_FlagRST | tcp_FlagACK;

		tcp_Send(s);
	}

	s->unhappy = 0;

	s->dataSize = 0;

	s->state = tcp_StateCLOSED;

	s->dataHandler(s, 0, -1);

	tcp_Unthread(s);

} /* void tcp_Abort(...) */

/* Retransmitter - called periodically to perform tcp retransmissions */
static void tcp_Retransmitter()
{
tcp_Socket *s;

BOOL x;

	for ( s = tcp_allsocs; s; s = s->next )
	{
		x = false;

		if ( s->dataSize > 0 || s->unhappy )
		{
			tcp_Send(s);

			x = true;
		}
		if ( x || s->state != tcp_StateESTAB )

			s->timeout -= tcp_RETRANSMITTIME;

		if ( s->timeout <= 0 )
		{
			if ( s->state == tcp_StateTIMEWT )
			{
#ifndef NO_POSIX
				printf("Closed.    \n");
#else
				;
#endif /* NO_POSIX */
				s->state = tcp_StateCLOSED;

				s->dataHandler(s, 0, 0);

				tcp_Unthread(s);
			}
			else
			{
#ifndef NO_POSIX
				printf("Timeout, aborting\n");
#else
				;
#endif /* NO_POSIX */
				tcp_Abort(s);
			}
		}
	}

} /* void tcp_Retransmitter()*/

/* Write data to a connection. Returns number of bytes written or '0' when connection is not in established state */
static int tcp_Write(tcp_Socket * s, unsigned char * dp, int * len)
{
int x;
#if (0)

	if ( s->state != tcp_StateESTAB )

		(int)(*len) = 0; 

	if ( (int)(*len) > (x = tcp_MaxData - s->dataSize) )  

		(int)(*len) = x; 

	if ( (int)(*len) > 0 )
	{
		Move(dp, &s->data[s->dataSize], (int)(*len) );

		s->dataSize += (int)(*len);

		tcp_Flush(s);
	}

    return ( (int)(*len) );
#else
int llen = *len;	

	if ( s->state != tcp_StateESTAB )

		llen = 0;

	if ( llen > (x = tcp_MaxData - s->dataSize) )

		llen = x;

	if ( llen > 0 )
	{
		Move(dp, &s->data[s->dataSize], llen);

		s->dataSize += llen;

		tcp_Flush(s);
	}

	return ( llen );
#endif /* (0) */
}
/*  void tcp_Write(. . .) */

/* void Send pending data */
static void tcp_Flush(tcp_Socket *s)
{
	if ( s->dataSize > 0 )
	{
		s->flags |= tcp_FlagPUSH;

		tcp_Send(s);
	}

} /* void tcp_Flush(...) */

/* Unthread a socket from the socket list, if it's there */
static void tcp_Unthread(tcp_Socket * ds)
{
tcp_Socket *s, **sp;

	sp = &tcp_allsocs;

	for (;;)
	{
		s = *sp;

		if ( s == ds )
		{
			*sp = s->next;

			break;
		}

		if (NULL ==  s)

			break;

		sp = &s->next;
	}

} /* void tcp_Unthread(. . .) */


/* Handler for incoming packets. */
static void tcp_Handler(in_Header * ip)
{
tcp_Header *tp;

tcp_PseudoHeader ph;

int len;

unsigned char *dp;

int x, diff;

tcp_Socket *s;

unsigned short flags;

len = in_GetHdrlenBytes(ip);

tp = (tcp_Header *)((unsigned char *)ip + len);

len = ip->length - len;

	/* demux to active sockets */
	for ( s = tcp_allsocs; s; s = s->next )

		if ( s->hisport != 0 && tp->dstPort == s->myport && tp->srcPort == s->hisport && ip->source == s->hisaddr )

			break;

	if ( NULL == s)
	{
		for ( s = tcp_allsocs; s; s = s->next )

			if ( s->hisport == 0 && tp->dstPort == s->myport )

				break;
	}

	if (NULL == s)
	{
#ifdef DEBUG_INFO
		if ( tcp_logState )

			tcp_DumpHeader(ip, tp, "Discarding");
#endif /* DEBUG_INFO */
		return;
	}

#ifdef DEBUG_INFO
	if ( tcp_logState )

		tcp_DumpHeader(ip, tp, "Received");
#endif /* DEBUG_INFO */

	/* save peer's ethernet address */
	MoveW(&((((eth_Header *)ip) - 1)->source[0]), &s->hisethaddr[0], sizeof(eth_HwAddress));

#ifdef DEBUG_INFO
#ifndef NO_POSIX
	printf ("His eth addr: 0x%04x 0x%04x 0x%04x\n", s->hisethaddr[0], s->hisethaddr[1], s->hisethaddr[2]);
#else
	;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */

	ph.src = ip->source;

	ph.dst = ip->destination;

	ph.mbz = 0;

	ph.protocol = 6;

	ph.length = len;

	ph.checksum = checksum(tp, len);

	if ( checksum(&ph, sizeof ph) != 0xffff  ) 
#ifdef DEBUG_INFO
#ifndef NO_POSIX
        printf("bad tcp checksum (0x%04x), received anyway\n", checksum(&ph, sizeof ph));
#else
	;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */

	flags = tp->flags;

	if ( flags & tcp_FlagRST )
	{
#ifndef NO_POSIX
		printf("connection reset\n");
#else
		;
#endif /* NO_POSIX */		

		s->state = tcp_StateCLOSED;

		s->dataHandler(s, 0, -1);

		tcp_Unthread(s);

#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: flags & tcp_FlagRST\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		return;
	}

	switch ( s->state )
	{

	case tcp_StateLISTEN:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateLISTEN\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		if ( flags & tcp_FlagSYN )
		{
			s->acknum = tp->seqnum + 1;

			s->hisport = tp->srcPort;

			s->hisaddr = ip->source;

			s->flags = tcp_FlagSYN | tcp_FlagACK;

			tcp_Send(s);

			s->state = tcp_StateSYNREC;

			s->unhappy = true;

			s->timeout = tcp_TIMEOUT;

#ifndef NO_POSIX
			printf("Syn from 0x%x#%d (seq 0x%x)\n", s->hisaddr, s->hisport, tp->seqnum);
#else
			;
#endif /* NO_POSIX */

		}
		break;

	case tcp_StateSYNSENT:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateSYNSENT \n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		if ( flags & tcp_FlagSYN )
		{
			s->acknum++;

			s->flags = tcp_FlagACK;

			s->timeout = tcp_TIMEOUT;

			if ( (flags & tcp_FlagACK) && tp->acknum == (s->seqnum + 1) )
			{
#ifndef NO_POSIX
				printf("Open\n");
#else
				;
#endif /* NO_POSIX */
				s->state = tcp_StateESTAB;

				s->seqnum++;

				s->acknum = tp->seqnum + 1;

				s->unhappy = false;
			}
			else
			{
				s->state = tcp_StateSYNREC;
			}
		}
		break;

	case tcp_StateSYNREC:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateSYNREC\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		if ( flags & tcp_FlagSYN )
		{
			s->flags = tcp_FlagSYN | tcp_FlagACK;

			tcp_Send(s);

			s->timeout = tcp_TIMEOUT;

#ifndef NO_POSIX
			printf(" retransmit of original syn\n");
#else
			;
#endif /* NO_POSIX */
		}

		if ( (flags & tcp_FlagACK) && tp->acknum == (s->seqnum + 1) )
		{
			s->flags = tcp_FlagACK;

			tcp_Send(s);

			s->seqnum++;

			s->unhappy = false;

			s->state = tcp_StateESTAB;

			s->timeout = tcp_TIMEOUT;

#ifndef NO_POSIX
			printf("Synack received - connection established\n");
#else
			;
#endif /* NO_POSIX */

		}
		break;

	case tcp_StateESTAB:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateESTAB\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		if ( (flags & tcp_FlagACK) == 0 ) 

			return;

		/* process ack value in packet */
		diff = tp->acknum - s->seqnum;

		if ( diff > 0 )
		{
			Move(&s->data[diff], &s->data[0], diff);

			s->dataSize -= diff;

			s->seqnum += diff;
		}

		s->flags = tcp_FlagACK;

		tcp_ProcessData(s, tp, len);

		break;

	case tcp_StateFINWT1:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateFINWT1\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		if ( (flags & tcp_FlagACK) == 0 )

			return;

		diff = tp->acknum - s->seqnum - 1;

		s->flags = tcp_FlagACK | tcp_FlagFIN;

		if ( diff == 0 )
		{
			s->state = tcp_StateFINWT2;

			s->flags = tcp_FlagACK;

#ifndef NO_POSIX
			printf("finack received.\n");
#else
			;
#endif /* NO_POSIX */
		}

		tcp_ProcessData(s, tp, len);

		break;

	case tcp_StateFINWT2:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateFINWT2\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		s->flags = tcp_FlagACK;

		tcp_ProcessData(s, tp, len);

		break;

	case tcp_StateCLOSING:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateCLOSING\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		if ( tp->acknum == (s->seqnum + 1) )
		{
			s->state = tcp_StateTIMEWT;

			s->timeout = tcp_TIMEOUT;
		}
		break;

	case tcp_StateLASTACK:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateCLOSING\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		if ( tp->acknum == (s->seqnum + 1) )
		{
			s->state = tcp_StateCLOSED;

			s->unhappy = false;

			s->dataSize = 0;

			s->dataHandler(s, 0, 0);

			tcp_Unthread(s);

#ifndef NO_POSIX
			printf("Closed.    \n");
#else
			;
#endif /* NO_POSIX */
		}
		else
		{
			s->flags = tcp_FlagACK | tcp_FlagFIN;

			tcp_Send(s);

			s->timeout = tcp_TIMEOUT;

#ifndef NO_POSIX
			printf("retransmitting FIN\n");
#else
			;
#endif /* NO_POSIX */
		}
		break;

	case tcp_StateTIMEWT:
#ifdef DEBUG_INFO
#ifndef NO_POSIX
		printf("tcp_Handl.: tcp_StateTIMEWT\n");
#else
		;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
		s->flags = tcp_FlagACK;

		tcp_Send(s);
    }

} /* void tcp_Handler(. . .) */

/* Process the data in an incoming packet. Called from all states where incoming data can be received: established, fin-wait-1, fin-wait-2 */
static void tcp_ProcessData(tcp_Socket * s, tcp_Header * tp, int len)
{
int diff, x;

unsigned short flags;

unsigned char *dp;

	flags = tp->flags;

	diff = s->acknum - tp->seqnum;

	if ( flags & tcp_FlagSYN )

		diff--;

	x = tcp_GetDataOffset(tp) << 2;

	dp = (unsigned char *)tp + x;

	len -= x;

	if ( diff >= 0 )
	{
		dp += diff;

		len -= diff;

		s->acknum += len;

		if (s->dataHandler)

			s->dataHandler(s, dp, len);
#ifdef DEBUG_INFO
		else 
#ifndef NO_POSIX
			printf("zero d-handler. \n");
#else
			;
#endif /* NO_POSIX */
#endif/* DEBUG_INFO */

		if ( flags & tcp_FlagFIN )
		{
			s->acknum++;

#ifdef DEBUG_INFO
#ifndef NO_POSIX
			printf("consumed fin.\n");
#else
			;
#endif /* NO_POSIX */
#endif/* DEBUG_INFO */
			switch(s->state)
			{
			case tcp_StateESTAB:
				/* note: skip state CLOSEWT by automatically closing conn */
				x = tcp_StateLASTACK;

				s->flags |= tcp_FlagFIN;

				s->unhappy = true;
#ifdef DEBUG_INFO
#ifndef NO_POSIX
				printf("sending fin.\n");
#else
				;
#endif /* NO_POSIX */
#endif /* DEBUG_INFO */
				break;

			case tcp_StateFINWT1:

				x = tcp_StateCLOSING;

				break;

			case tcp_StateFINWT2:

				x = tcp_StateTIMEWT;

				break;
			}
			s->state = x;
		}
	}

	s->timeout = tcp_TIMEOUT;

	tcp_Send(s);

} /* void tcp_ProcessData(. . .) */

/* Format and send an outgoing segment */
static void tcp_Send(tcp_Socket * s)
{
tcp_PseudoHeader ph;

struct _pkt
{
	in_Header in;

	tcp_Header tcp;

	unsigned long maxsegopt;
} *pkt;

unsigned char *dp;

	pkt = (struct _pkt *)sed_FormatPacket(&(s->hisethaddr[0]), 0x800);	

	dp = (unsigned char*)(&pkt->maxsegopt);

	pkt->in.length = sizeof(in_Header) + sizeof(tcp_Header) + s->dataSize;

	/* tcp header */
	pkt->tcp.srcPort = s->myport;

	pkt->tcp.dstPort = s->hisport;

	pkt->tcp.seqnum = s->seqnum;

	pkt->tcp.acknum = s->acknum;

	pkt->tcp.window = 1024;

	pkt->tcp.flags = s->flags | 0x5000;

	pkt->tcp.checksum = 0;

	pkt->tcp.urgentPointer = 0;

	if ( s->flags & tcp_FlagSYN )
	{
		pkt->tcp.flags += 0x1000;

		pkt->in.length += 4;

		pkt->maxsegopt = 0x02040578; /* 1400 bytes */

		dp += 4;
	}

	MoveW(s->data, dp, s->dataSize);

	/* internet header - version 4, hdrlen 5, tos  */
	pkt->in.vht = 0x4500;

	pkt->in.identification = tcp_id++;

	pkt->in.frag = 0;

	pkt->in.ttlProtocol = (250<<8) + 6;

	pkt->in.checksum = 0;

	pkt->in.source = sin_lclINAddr;

	pkt->in.destination = s->hisaddr;

	pkt->in.checksum = ~checksum(&pkt->in, sizeof(in_Header));

	/* compute tcp checksum */
	ph.src = pkt->in.source;

	ph.dst = pkt->in.destination;

	ph.mbz = 0;

	ph.protocol = 6;

	ph.length = pkt->in.length - sizeof(in_Header);

	ph.checksum = checksum(&pkt->tcp, ph.length);

	pkt->tcp.checksum = ~checksum(&ph, sizeof ph);

#ifdef DEBUG_INFO
	if ( tcp_logState )

		tcp_DumpHeader(&pkt->in, &pkt->tcp, "Sending");
#endif /* DEBUG_INFO */

    sed_Send(pkt->in.length);

} /* void tcp_Send(. . .) */

/* Do a one's complement checksum */
static unsigned long checksum(unsigned short * dp, int length)
{
int len;

unsigned long sum;

	len = length >> 1;

	sum = 0;

	while ( len-- > 0 )

		sum += *dp++;

	if ( length & 1 )

		sum += (*dp & 0xFF00);

	sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);

	sum = (sum & 0xFFFF) + ((sum >> 16) & 0xFFFF);

	return ( sum );

} /* unsigned long checksum(. . .) */

/* Dump the tcp protocol header of a packet */
static void tcp_DumpHeader(in_Header * ip, tcp_Header * tp, unsigned char * mesg )
{
static char *flags[] = { "FIN", "SYN", "RST", "PUSH", "ACK", "URG" };

int len;

unsigned short f;

	tp = (tcp_Header *)((unsigned char *)ip + in_GetHdrlenBytes(ip));    

	len =  ip->length - ((tcp_GetDataOffset(tp) + in_GetHdrlen(ip)) << 2);

#ifndef NO_POSIX
	printf("TCP: %s packet:\nS: %x; D: %x; SN=%x ACK=%x W=%d DLen=%d\n",  mesg, tp->srcPort, tp->dstPort, tp->seqnum, tp->acknum,  tp->window, len);
#else
	;
#endif /* NO_POSIX */


#ifndef NO_POSIX
	printf("DO=%d, C=%x U=%d", tcp_GetDataOffset(tp), tp->checksum, tp->urgentPointer);
#else
	;
#endif /* NO_POSIX */


	/* output flags */
	f = tp->flags;

	for ( len = 0; len < 6; len++ )

		if ( f & (1 << len) )
#ifndef NO_POSIX
			printf(" %s", flags[len]);
#else
			;
#endif /* NO_POSIX */

	printf("\n");

} /* void tcp_DumpHeader(. . .)*/

/* Task-oriented TCP packet print-out  */
static void __tcp_DumpHeader( )
{
static char *flags[] = { "FIN", "SYN", "RST", "PUSH", "ACK", "URG" };

int len;

unsigned short f;

in_Header *ip = floating_ip_buffer;

register tcp_Header *tp;

	tp = (tcp_Header *)((unsigned char *)floating_ip_buffer + in_GetHdrlenBytes(ip));    

	if (tp)
	{
		len =  ip->length - ((tcp_GetDataOffset(tp) + in_GetHdrlen(ip)) << 2);

#ifndef NO_POSIX
		printf("the pack: S: %x; D: %x; SN=%x ACK=%x W=%d DLen=%d ", tp->srcPort, tp->dstPort, tp->seqnum, tp->acknum,tp->window, len);
#else
		;
#endif /* NO_POSIX */


#ifndef NO_POSIX
		printf("DO=%d, C=%x U=%d", tcp_GetDataOffset(tp), tp->checksum, tp->urgentPointer);
#else
		;
#endif /* NO_POSIX */

		/* output flags */
		f = tp->flags;

		for ( len = 0; len < 6; len++ )

			if ( f & (1 << len) )

				printf(" %s", flags[len]);

#ifndef NO_POSIX
		printf("\n");
#else
		;
#endif /* NO_POSIX */

	}

} /* __tcp_DumpHeader(. . .)*/

/* Move bytes from hither to yon */
static void Move(unsigned char * src, unsigned char * dest, unsigned long numbytes )
{
	if ( numbytes <= 0 )

		return;

	if ( src < dest )
	{
		src += numbytes;

		dest += numbytes;

		do
		{
			*--dest = *--src;

		} while ( --numbytes > 0 );
	}
	else do
	{
		*dest++ = *src++;

	} while ( --numbytes > 0 );

} /* void Move(. . .)*/

/* Do an address resolution bit. Returns 1 if did , 0 otherwise */
static int sar_MapIn2Eth(unsigned long ina, eth_HwAddress * ethap)
{	
    return ( 0 );

} /* int sar_MapIn2Eth(. . .) */

/* Format an ethernet header in the transmit buffer */
static unsigned char * sed_FormatPacket( unsigned short * destEAddr, unsigned short ethType )
{
unsigned char * xMitBuf;

	xMitBuf = floating_socket_buffer;

	/* we need to clean the IP buffer  to renew (tcp_MaxData + all-headers-area) */
	memset ( xMitBuf,0, sizeof (eth_Header) + sizeof (in_Header) + sizeof (tcp_Header));


	Move( (unsigned char*)destEAddr, xMitBuf, 6 );

	Move( (unsigned char*)sed_lclEthAddr, xMitBuf + 6, 6 );

	*((short *)(xMitBuf+12)) = ethType;

#ifdef DUMP_PACKET
	{
	int _i = 99;

#ifndef NO_POSIX
		printf("in sed_FormatPacket(). \n");
#else
		;
#endif /* NO_POSIX */


		for (; _i>=0; _i-- )
#ifndef NO_POSIX
			printf ("  %02x", xMitBuf[99-_i]);
#else
			;
#endif /* NO_POSIX */


#ifndef NO_POSIX
		printf ("\n");
#else
		;
#endif /* NO_POSIX */

	}
#endif /* DEBUG_INFO */

    return ( xMitBuf+14 );

} /* unsigned char * sed_FormatPacket(. . .) */

/* Send a packet out over the ethernet. Packet at thebeginning of the transmit buffer.  Returns when the packet has been successfully sent */
static void sed_Send(int pkLengthInOctets )
{
	;
} /* void sed_Send(. . .)*/

/* Test for the arrival of a packet on the Ethernet interface */
unsigned char * sed_IsPacket()
{
    /* recall: sizeof(eth_Header) = 16 */
    return (unsigned char*)((unsigned long)floating_socket_buffer+sizeof(eth_Header));

} /* unsigned char * sed_IsPacket() */

/* Check to make sure that the packet that you received was the one that you expected to get */
static int sed_CheckPacket( unsigned short * recBufLocation, unsigned short expectedType )
{
unsigned long recHeader;

	recHeader	 = recBufLocation[-8];

	if ( (recHeader&R_ERROR) != 0 || (recHeader&R_OFFSET) < E10P_MIN )
	{
		return ( -1 );
	}

	if ( recBufLocation[-1] != expectedType )
	{
		return ( 0 );
	}

	return (1);

} /* int sed_CheckPacket(. . .)*/

/* Process packat. Returns 1 is processed, 0 otherwise */
static int sar_CheckPacket(arp_Header * ap)
{
arp_Header *op;

	/* Is it: not ethernet hardware, not internet software, not a resolution request, not addresses to me? */
	if ( ap->hwType != arp_TypeEther || ap->protType != 0x800 || ap->opcode != ARP_REQUEST || ap->dstIPAddr != sin_lclINAddr )

		/* Then ingnore this packet */	
		return ( 0 );

	/* Normal packet - prepare a  response */
	op = (arp_Header *)sed_FormatPacket(&(ap->srcEthAddr[0]), 0x806);

	op->hwType = arp_TypeEther;

	op->protType = 0x800;

	op->hwProtAddrLen = (sizeof(eth_HwAddress) << 8) + sizeof(in_HwAddress);

	op->opcode = ARP_REPLY;

	op->srcIPAddr = sin_lclINAddr;

	MoveW(sed_lclEthAddr, op->srcEthAddr, sizeof(eth_HwAddress));

	ap->dstIPAddr = op->srcIPAddr;

	MoveW(ap->srcEthAddr, op->dstEthAddr, sizeof(eth_HwAddress));

	sed_Send(sizeof(arp_Header));

	return ( 1 );

} /* int sar_CheckPacket(. . .) */
