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
 *
 */

#ifndef _TCP_H_
#define _TCP_H_

#define CLOCKS_PER_MILLISEC 	22

#define clock_ValueRough() 	(clock() * 22)

/* Interval at which retransmitter is called */
#define tcp_RETRANSMITTIME	1000	

/* Timeout for opens */
#define tcp_LONGTIMEOUT 	31000	

/* Timeout during a connection */
#define tcp_TIMEOUT		10000	

#ifdef DEBUG_INFO

#endif /* DEBUG_INFO */

#define true        		1

#define false       		0




#ifndef NO_POSIX


#else

/* TODO: adjust in accordance to what the Atollis' crosscompiler percepts */
#define size_t		int

/* TODO: check on 16-bit and 32-bit addresses */
#define NULL		((void*)0)

/* TODO: replace with platform's native 'printf()' */
//+++#define printf(const char *format, ...)   ;

#endif /* NO_POSIX */






/* Pointger onto fn. without parameters  returning int */
typedef int (*procref)();

/* Redef. boolean type locally */
typedef short BOOL;                  

/* Protocol address definition */
typedef unsigned long  in_HwAddress;

/* Hardware address definition */
typedef unsigned short eth_HwAddress[3];

/* The Ethernet header */
typedef struct
{
	eth_HwAddress   destination;

	eth_HwAddress   source;

	unsigned short  type;

} eth_Header;


/* The Internet Header: */
typedef struct
{
	/* version, hdrlen, tos */
	unsigned short  vht;

	unsigned short  length;

	unsigned short  identification;

	unsigned short  frag;

	unsigned short  ttlProtocol;

	unsigned short  checksum;

	in_HwAddress    source;

	in_HwAddress    destination;

} in_Header;

#define in_GetVersion(ip) (((ip)->vht >> 12) & 0xf)

#define in_GetHdrlen(ip)  (((ip)->vht >> 8) & 0xf)

#define in_GetHdrlenBytes(ip)  (((ip)->vht >> 6) & 0x3c)

#define in_GetTos(ip)      ((ip)->vht & 0xff)

#define in_GetTTL(ip)      ((ip)->ttlProtocol >> 8)

#define in_GetProtocol(ip) ((ip)->ttlProtocol & 0xff)


typedef struct
{
	unsigned short            srcPort;

	unsigned short            dstPort;

	unsigned long             seqnum;

	unsigned long             acknum;

	unsigned short            flags;

	unsigned short            window;

	unsigned short            checksum;

	unsigned short            urgentPointer;

} tcp_Header;


#define tcp_FlagFIN     0x0001

#define tcp_FlagSYN     0x0002

#define tcp_FlagRST     0x0004

#define tcp_FlagPUSH    0x0008

#define tcp_FlagACK     0x0010

#define tcp_FlagURG     0x0020

#define tcp_FlagDO      0xF000

#define tcp_GetDataOffset(tp) ((tp)->flags >> 12)

/* The TCP/UDP Pseudo Header */
typedef struct
{
	in_HwAddress    src;

	in_HwAddress    dst;

	unsigned char   mbz;

	unsigned char   protocol;

	unsigned short  length;

	unsigned short  checksum;

} tcp_PseudoHeader;

/* listening for connection */
#define tcp_StateLISTEN  0
  
/* syn sent, active open */
#define tcp_StateSYNSENT 1

/* syn received, synack+syn sent. */
#define tcp_StateSYNREC  2

/* established */
#define tcp_StateESTAB   3      

/* sent FIN */
#define tcp_StateFINWT1  4

/* sent FIN, received FINACK */
#define tcp_StateFINWT2  5
      
/* received FIN waiting for close; sent FIN, received FIN (waiting for FINACK); /* close-wait state is bypassed by automatically closing a connection when a FIN is received. */
#define tcp_StateCLOSING 6

/* fin received, finack+fin sent */
#define tcp_StateLASTACK 7 
/* dally after sending final FINACK */

#define tcp_StateTIMEWT  8      
/* finack received */
#define tcp_StateCLOSED  9      

/* maximum bytes to buffer on output */
#define tcp_MaxData 0x5000 /* formerly: 32 */

typedef struct _tcp_socket
{
	/* Next sock in a chain, */
	struct _tcp_socket *next;

	/* connection state */
	short           state;          

	/* called with incoming data */
	procref         dataHandler;    

	/* ethernet address of peer */
	eth_HwAddress   hisethaddr;     

	/* internet address of peer */
	in_HwAddress    hisaddr;        

	/* tcp ports for this connection */
	unsigned short  myport, hisport;

	/* data ack'd and sequence num */
	unsigned long   acknum, seqnum; 

	/* timeout, in milliseconds */
	int             timeout;        

	/* flag, indicates retransmitting segt's */
	BOOL            unhappy;        

	/* tcp flags word for last packet sent */
	unsigned short  flags;          

	/* number of bytes of data to send */
	short           dataSize;       

	/* data to send */
	unsigned char   data[tcp_MaxData]; 

} tcp_Socket;



/* ARP type of Ethernet address */
#define arp_TypeEther  1

/* Request opcode */
#define ARP_REQUEST 1

/* Responce op code */
#define ARP_REPLY   2

/* Arp header */
typedef struct
{
	unsigned short  hwType;

	unsigned short  protType;

	/* hw and prot addr len */
	unsigned short  hwProtAddrLen;  

	unsigned short  opcode;

	eth_HwAddress   srcEthAddr;

	in_HwAddress    srcIPAddr;

	eth_HwAddress   dstEthAddr;

	in_HwAddress    dstIPAddr;

} arp_Header;

/* was there any error */
#define	R_ERROR		0x8800	

/* packet length + 1 word */
#define	R_OFFSET	0x07FF	

/* Minimum Ethernet packet size */
#define E10P_MIN	60      



unsigned char * floating_socket_buffer;

int tcp_id, idOnAlfa/*, idOnBeta*/;

static eth_HwAddress sed_lclEthAddr;

static eth_HwAddress sed_ethBcastAddr;

static in_HwAddress  sin_lclINAddr;


static  in_Header * floating_ip_buffer;

tcp_Socket Socket1/*, Socket2*/;

unsigned short ushAlfaPort, ushBetaPort;

in_HwAddress AlfaIpAddress,  BetaIpAddress;

static  eth_HwAddress AlfaEthAddress, BetaEthAddress;

tcp_Socket *tcp_allsocs, *onAlfa/*, *onBeta*/;



/* Task-related initialization */
void tcp_Module_Init(void);

/* Initialize the tcp implementation on given machine */
void tcp_Init(eth_HwAddress _MyEthHwAddress, unsigned long uloMyIpAddress);

/* Actively open a TCP connection to a particular destination. */
unsigned long tcp_Open(tcp_Socket *s, unsigned short lport, in_HwAddress ina, unsigned short port, procref datahandler);

/* Passive open: listen for a connection on a particular port */
static void tcp_Listen(tcp_Socket *s, unsigned short port, procref datahandler, int timeout );

/* Send a FIN on a particular port -- only works if it is open */
void tcp_Close(tcp_Socket * s);

/* Abort a tcp connection */
static void tcp_Abort(tcp_Socket *s);

/* Retransmitter - called periodically to perform tcp retransmissions */
static void tcp_Retransmitter();

/* Write data to a connection. Returns number of bytes written or '0' when connection is not in established state */
static int tcp_Write(tcp_Socket * s, unsigned char * dp, int * len);

/* void Send pending data */
static void tcp_Flush(tcp_Socket *s);

/* Unthread a socket from the socket list, if it's there */
static void tcp_Unthread(tcp_Socket * ds);

/* Handler for incoming packets. */
static void tcp_Handler(in_Header * ip);

/* Process the data in an incoming packet. Called from all states where incoming data can be received: established, fin-wait-1, fin-wait-2 */
static void tcp_ProcessData(tcp_Socket * s, tcp_Header * tp, int len);

/* Format and send an outgoing segment */
static void tcp_Send(tcp_Socket * s);

/* Do a one's complement checksum */
static unsigned long checksum(unsigned short * dp, int length);

/* Dump the tcp protocol header of a packet */
static void tcp_DumpHeader(in_Header * ip, tcp_Header * tp, unsigned char * mesg );

/* Task-oriented TCP pack. dump  */
static void __tcp_DumpHeader( );

/* Move bytes from hither to yon */
static void Move(unsigned char * src, unsigned char * dest, unsigned long numbytes );

/* Do an address resolution bit. Returns 1 if did , 0 otherwise */
static int sar_MapIn2Eth(unsigned long ina, eth_HwAddress * ethap);

/* Format an ethernet header in the transmit buffer */
static unsigned char * sed_FormatPacket( unsigned short * destEAddr, unsigned short ethType );

/* Send a packet out over the ethernet. Packet at thebeginning of the transmit buffer.  Returns when the packet has been successfully sent */
static void sed_Send(int pkLengthInOctets );

/* Test for the arrival of a packet on the Ethernet interface */
unsigned char * sed_IsPacket();

/* Check to make sure that the packet that you received was the one that you expected to get */
static int sed_CheckPacket( unsigned short * recBufLocation, unsigned short expectedType );

/* Process packat. Returns 1 is processed, 0 otherwise */
static int sar_CheckPacket(arp_Header * ap);





#endif /* _TCP_H_ */
