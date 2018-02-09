#
# Copyright (c) 2017, arneri, arneri@ukr.net All rights reserved
#
# A 'client' part of HTTP-JSON
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#        * Redistributions of source code must retain the above copyright
#          notice, this list of conditions and the following disclaimer.
#        * Redistributions in binary form must reproduce the above copyright
#          notice, this list of conditions and the following disclaimer in the
#          documentation and/or other materials provided with the distribution.
#        * Neither the name of The Linux Foundation nor
#          the names of its contributors may be used to endorse or promote
#          products derived from this software without specific prior written
#          permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

ifeq ($(strip $(platform)),)
$(error "Define 'platform', as one of those: 'platform=pc', 'platform=f4' . Don't use capitals while defining a platform kind, don't confuse a syntax otherwise")
endif


TARGET=client

TCPDIR=./tcp

JSONDIR=./json

OBJS=$(JSONDIR)/json.o


ifeq ($(strip $(platform)),pc)
	CFLAGS=-O3  -DDEBUG_INFO -DDUMP_PACKET
# DEBUG_INFO	output of intermediate dsata frmo within TCP/IP stack
# DDUMP_PACKET	output of raw ATH packet (including IP payload which includes TCP payload)
	OBJS+=

	GRBG=$(JSONDIR)/*~ *~ $(TARGET) $(OBJS)
	EXTRA=
else
	ifeq ($(strip $(platform)),f4)
		# STM32F4 crosscompiler (actually whatever Atollic TRUE Studio suggests, but normally it should be "architecture-firm-baremetal-executableformat-" )
		PREFIX=
		CFLAGS= -DNO_POSIX
		OBJS+=$(TCPDIR)/tcp.o

		GRBG=$(TCPDIR)/*~ $(JSONDIR)/*~  *~ $(TARGET) $(OBJS)
		EXTRA=
	endif
endif

TARGET=client


%.o: %.c
	$(CC)  $(CFLAGS)  -c -o  $@ $<

$(TARGET): $(TARGET).c $(OBJS)
	$(PREFIX)$(CC)  $(CFLAGS) $(LDFLAGS) -o $@ $^


clean:
	rm -f  $(GRBG) 

