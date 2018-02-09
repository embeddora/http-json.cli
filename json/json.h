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


#ifndef _JSON_H_
#define _JSON_H_


#ifndef NO_POSIX
#include <stddef.h>
#else
/* TODO: adjust in accordance to what the Atollis' crosscompiler percepts */
#define size_t		int

/* TODO: check on 16-bit and 32-bit addresses */
#define NULL		((void*)0)

#endif /* NO_POSIX */


/* JSON type identifier */
typedef enum
{
	JSMN_UNDEFINED = 0,

	/* Object */
	JSMN_OBJECT = 1,

	/* Array */
	JSMN_ARRAY = 2,

	/* String */
	JSMN_STRING = 3,

	/* Other primitive: number, boolean (true/false) or null */
	JSMN_PRIMITIVE = 4

} jsmntype_t;

enum jsmnerr
{
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,

	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,

	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3
};

typedef struct
{

	/* Type: object, array, string, etc */
	jsmntype_t type;

	/* Start position in JSON data string */
	int start;

	/* End position in JSON data string */
	int end;

	/* The size of item */
	int size;

	int parent;

} jsmntok_t;

/* JSON parser. Contains an array of token blocks available. Also stores the string being parsed now and current position in that string */
typedef struct
{
	/* offset in the JSON string */
	unsigned int pos;

	/* next token to allocate */
	unsigned int toknext;

	/* superior token node, e.g parent object or array */
	int toksuper; 

} jsmn_parser;


/* Create JSON parser over an array of tokens */
void jsmn_init(jsmn_parser *parser);

/* Run JSON parser. It parses a JSON data string into and array of tokens, each describing a single JSON object */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len, jsmntok_t *tokens, unsigned int num_tokens);


#endif /* _JSON_H_ */
